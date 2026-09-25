// Declares the SDL, OpenGL, and Dear ImGui runtime owner.
#pragma once

#include <SDL.h>

namespace app::runtime
{
    // Owns SDL, OpenGL, and ImGui and releases them in reverse initialization order.
    class SdlImGuiRuntime
    {
    public:
        // Creates an uninitialized runtime.
        SdlImGuiRuntime() = default;
        // Releases any initialized platform resources.
        ~SdlImGuiRuntime();

        // Runtime instances own unique platform handles and cannot be copied.
        SdlImGuiRuntime(const SdlImGuiRuntime&) = delete;
        // Runtime instances own unique platform handles and cannot be copied.
        SdlImGuiRuntime& operator=(const SdlImGuiRuntime&) = delete;

        // Initializes SDL, the graphics context, and Dear ImGui in dependency order.
        bool initialize();
        // Releases initialized components in reverse dependency order.
        void shutdown();

        // Processes platform events and returns false when the application should close.
        [[nodiscard]] bool process_events();
        /// Starts a new Dear ImGui frame.
        void begin_frame();
        // Renders and presents the current Dear ImGui frame.
        void end_frame();

    private:
        SDL_Window* window_ = nullptr;
        SDL_GLContext gl_context_ = nullptr;
        bool sdl_initialized_ = false;
        bool text_input_started_ = false;
        bool imgui_context_created_ = false;
        bool sdl_backend_initialized_ = false;
        bool opengl_backend_initialized_ = false;
    };
}
