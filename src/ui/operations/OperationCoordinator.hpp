/// @file
/// Declares coordination between UI state and asynchronous operations.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace app
{
    namespace operations
    {
        class OperationController;
    }
}

namespace ui::operations
{
    struct OperationState;

    /// Converts UI requests and client-operation results into an OperationState.
    class OperationCoordinator
    {
    public:
        /// Connects the coordinator to the application operation controller.
        explicit OperationCoordinator(app::operations::OperationController& controller);

        /// Clears output and pending bookkeeping without changing input options.
        void clear(OperationState& state);
        /// Cancels the active primary operation and updates visible state.
        void cancel(OperationState& state);
        /// Transfers any completed controller result into UI state.
        void poll(OperationState& state);

        /// Starts translation using the current automaton display mode.
        void start_translate(
            OperationState& state, std::string_view regex, std::string_view extra_alphabet
        );
        /// Starts rewriting using the current expansion options.
        void start_rewrite(
            OperationState& state, std::string_view regex, std::string_view extra_alphabet
        );
        /// Starts language comparison for both supplied expressions.
        void start_compare(
            OperationState& state,
            std::string_view left_regex,
            std::string_view right_regex,
            std::string_view extra_alphabet
        );

        /// Returns whether a primary operation is active.
        [[nodiscard]] bool running() const;

    private:
        /// Tracks whether the controller is handling a primary operation.
        enum class PendingKind : std::uint8_t
        {
            None,
            PrimaryOperation
        };

        /// Replaces the current output with a pending-operation message.
        void begin(OperationState& state, std::string label);

        app::operations::OperationController& controller_;
        PendingKind pending_kind_ = PendingKind::None;
    };
}
