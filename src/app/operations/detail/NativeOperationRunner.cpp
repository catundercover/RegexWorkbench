// Implements isolated native operations through a child process and pipes.
#include "app/operations/OperationRunner.hpp"
#include "app/operations/detail/PlatformRunnerFactory.hpp"
#include "worker/protocol/OperationProtocol.hpp"
#include "worker/protocol/PayloadFrame.hpp"
#include "worker/protocol/ProtocolLimits.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <spawn.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <variant>
#include <vector>

namespace app::operations::detail
{
    namespace
    {
        // Wraps a malformed worker response in the operation result model.
        [[nodiscard]] OperationResult protocol_failure(const std::string& detail)
        {
            return OperationFailure{"The operation worker returned invalid data: " + detail};
        }

        // Locates the worker beside the application, with the current directory as fallback.
        [[nodiscard]] std::filesystem::path worker_path()
        {
            std::array<char, PATH_MAX> executable{};
            const ssize_t length =
                readlink("/proc/self/exe", executable.data(), executable.size() - 1);
            if (length > 0)
            {
                const auto path_length = static_cast<std::size_t>(length);
                return std::filesystem::path(std::string(executable.data(), path_length))
                           .parent_path() /
                       REGEXTHESIS_OPERATION_WORKER_NAME;
            }
            return std::filesystem::current_path() / REGEXTHESIS_OPERATION_WORKER_NAME;
        }

        // Writes a complete buffer while retrying interrupted system calls.
        bool write_all(const int descriptor, const std::span<const std::uint8_t> data)
        {
            std::size_t written = 0;
            while (written < data.size())
            {
                const ssize_t count =
                    write(descriptor, data.data() + written, data.size() - written);
                if (count < 0 && errno == EINTR)
                {
                    continue;
                }
                if (count <= 0)
                {
                    return false;
                }
                written += static_cast<std::size_t>(count);
            }
            return true;
        }

        // Runs one operation in a killable child process using framed pipe messages.
        class NativeOperationRunner final : public OperationRunner
        {
        public:
            // Terminates and reaps an active worker process.
            ~NativeOperationRunner() override
            {
                cancel();
            }

            // Serializes a request, spawns the worker, and writes its complete frame.
            bool start(OperationRequest request) override
            {
                if (is_running())
                {
                    return false;
                }

                worker::protocol::Payload payload;
                try
                {
                    payload = worker::protocol::encode_request(request);
                }
                catch (...)
                {
                    return false;
                }

                int request_pipe[2] = {-1, -1};
                int response_pipe[2] = {-1, -1};
                if (pipe(request_pipe) != 0 || pipe(response_pipe) != 0)
                {
                    close_pair(request_pipe);
                    close_pair(response_pipe);
                    return false;
                }
                if (!spawn_worker(request_pipe, response_pipe))
                {
                    close_pair(request_pipe);
                    close_pair(response_pipe);
                    return false;
                }

                close(request_pipe[0]);
                close(response_pipe[1]);
                response_descriptor_ = response_pipe[0];

                const worker::protocol::PayloadFrameHeader header =
                    worker::protocol::encode_payload_size(payload.size());
                const bool request_written =
                    write_all(request_pipe[1], header) && write_all(request_pipe[1], payload);
                close(request_pipe[1]);
                if (!request_written)
                {
                    cancel();
                    return false;
                }

                // Polling must never block the UI thread while the worker is computing.
                if (const int flags = fcntl(response_descriptor_, F_GETFL, 0);
                    flags < 0 || fcntl(response_descriptor_, F_SETFL, flags | O_NONBLOCK) != 0)
                {
                    cancel();
                    return false;
                }

                response_.clear();
                expected_size_.reset();
                running_ = true;
                return true;
            }

            // Accumulates nonblocking response bytes and returns one complete result.
            std::optional<OperationResult> poll() override
            {
                if (!running_)
                {
                    return std::nullopt;
                }

                read_available();
                if (!expected_size_ && response_.size() >= worker::protocol::PayloadFrameHeaderSize)
                {
                    worker::protocol::PayloadFrameHeader header{};
                    std::copy_n(response_.begin(), header.size(), header.begin());
                    expected_size_ = worker::protocol::decode_payload_size(header);
                    if (*expected_size_ > worker::protocol::limits::MaxPayloadBytes)
                    {
                        return finish_with_failure("the response is too large.");
                    }
                }

                if (expected_size_ &&
                    response_.size() >= *expected_size_ + worker::protocol::PayloadFrameHeaderSize)
                {
                    const std::size_t framed_size =
                        *expected_size_ + worker::protocol::PayloadFrameHeaderSize;
                    if (response_.size() != framed_size)
                    {
                        return finish_with_failure("the response contains trailing data.");
                    }

                    const std::span payload(
                        response_.data() + worker::protocol::PayloadFrameHeaderSize, *expected_size_
                    );
                    auto decoded = worker::protocol::decode_result(payload);
                    finish_process(false);
                    if (const auto* error = std::get_if<worker::protocol::ProtocolError>(&decoded))
                    {
                        return protocol_failure(error->message);
                    }
                    return std::get<OperationResult>(std::move(decoded));
                }

                int status = 0;
                const pid_t wait_result = waitpid(process_, &status, WNOHANG);
                if (wait_result == process_)
                {
                    process_ = -1;
                    close_response();
                    running_ = false;
                    return protocol_failure("the worker exited without a complete response.");
                }
                return std::nullopt;
            }

            // Kills the child process and discards its partial response.
            void cancel() override
            {
                finish_process(true);
                response_.clear();
                expected_size_.reset();
            }

            // Returns whether a child process is expected to produce a response.
            [[nodiscard]] bool is_running() const noexcept override
            {
                return running_;
            }

        private:
            // Closes every valid descriptor in a pipe pair and marks it invalid.
            static void close_pair(int (&descriptors)[2])
            {
                for (int& descriptor : descriptors)
                {
                    if (descriptor >= 0)
                    {
                        close(descriptor);
                        descriptor = -1;
                    }
                }
            }

            // Spawns the worker with its standard streams connected to the supplied pipes.
            bool spawn_worker(const int (&request_pipe)[2], const int (&response_pipe)[2])
            {
                posix_spawn_file_actions_t actions;
                if (posix_spawn_file_actions_init(&actions) != 0)
                {
                    return false;
                }

                const bool actions_ready =
                    posix_spawn_file_actions_adddup2(&actions, request_pipe[0], STDIN_FILENO) ==
                        0 &&
                    posix_spawn_file_actions_adddup2(&actions, response_pipe[1], STDOUT_FILENO) ==
                        0 &&
                    posix_spawn_file_actions_addclose(&actions, request_pipe[0]) == 0 &&
                    posix_spawn_file_actions_addclose(&actions, request_pipe[1]) == 0 &&
                    posix_spawn_file_actions_addclose(&actions, response_pipe[0]) == 0 &&
                    posix_spawn_file_actions_addclose(&actions, response_pipe[1]) == 0;

                int spawn_result = -1;
                if (actions_ready)
                {
                    const std::string executable = worker_path().string();
                    std::array<char*, 2> arguments = {
                        const_cast<char*>(executable.c_str()), nullptr
                    };
                    spawn_result = posix_spawn(
                        &process_, executable.c_str(), &actions, nullptr, arguments.data(), environ
                    );
                }
                posix_spawn_file_actions_destroy(&actions);

                if (spawn_result != 0)
                {
                    process_ = -1;
                    return false;
                }
                return true;
            }

            // Closes the parent side of the response pipe.
            void close_response()
            {
                if (response_descriptor_ >= 0)
                {
                    close(response_descriptor_);
                    response_descriptor_ = -1;
                }
            }

            // Optionally kills, then reaps, the child and restores idle state.
            void finish_process(const bool terminate)
            {
                close_response();
                if (process_ > 0)
                {
                    if (terminate)
                    {
                        kill(process_, SIGKILL);
                    }
                    int status = 0;
                    while (waitpid(process_, &status, 0) < 0 && errno == EINTR)
                    {
                    }
                    process_ = -1;
                }
                running_ = false;
            }

            // Reads all currently available response bytes without blocking.
            void read_available()
            {
                std::array<std::uint8_t, 8192> chunk{};
                while (response_descriptor_ >= 0)
                {
                    const ssize_t count = read(response_descriptor_, chunk.data(), chunk.size());
                    if (count > 0)
                    {
                        const auto end = chunk.begin() + static_cast<std::size_t>(count);
                        response_.insert(response_.end(), chunk.begin(), end);
                        if (response_.size() > worker::protocol::limits::MaxPayloadBytes +
                                                   worker::protocol::PayloadFrameHeaderSize)
                        {
                            break;
                        }
                        continue;
                    }
                    if (count < 0 && errno == EINTR)
                    {
                        continue;
                    }
                    break;
                }
            }

            // Terminates the worker and returns a protocol failure result.
            std::optional<OperationResult> finish_with_failure(const std::string& detail)
            {
                finish_process(true);
                return protocol_failure(detail);
            }

            pid_t process_ = -1;
            int response_descriptor_ = -1;
            bool running_ = false;
            std::optional<std::size_t> expected_size_;
            std::vector<std::uint8_t> response_;
        };
    }

    std::unique_ptr<OperationRunner> make_platform_runner()
    {
        return std::make_unique<NativeOperationRunner>();
    }
}
