// Implements SDL, OpenGL, and Dear ImGui resource management.
#include "app/runtime/SdlImGuiRuntime.hpp"

#include <SDL_opengles2.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <string>

namespace app::runtime
{
    namespace
    {
        // Resolves a bundled font in the native executable or virtual web filesystem.
        [[nodiscard]] std::string bundled_font_path(const char* filename)
        {
#ifdef __EMSCRIPTEN__
            return std::string("fonts/") + filename;
#else
            char* base_path = SDL_GetBasePath();
            const std::string path = base_path != nullptr ? base_path : "";
            SDL_free(base_path);
            return path + "fonts/" + filename;
#endif
        }

        // Loads the primary font and merges mathematical symbols into its atlas.
        void configure_fonts(ImGuiIO& io)
        {
            static constexpr ImWchar BaseRanges[] = {
                0x0020, 0x00FF, 0x0370, 0x03FF, 0x2600, 0x26FF, 0
            };
            static constexpr ImWchar MathRanges[] = {0x2200, 0x22FF, 0};

            io.Fonts->Clear();
            const std::string regular_font = bundled_font_path("NotoSans-Regular.ttf");
            ImFont* font =
                io.Fonts->AddFontFromFileTTF(regular_font.c_str(), 18.0f, nullptr, BaseRanges);

            ImFontConfig math_config;
            math_config.MergeMode = true;
            math_config.PixelSnapH = true;
            const std::string math_font = bundled_font_path("NotoSansMath-Regular.ttf");
            io.Fonts->AddFontFromFileTTF(math_font.c_str(), 18.0f, &math_config, MathRanges);

            if (font != nullptr)
            {
                io.FontDefault = font;
            }
        }

    }

    SdlImGuiRuntime::~SdlImGuiRuntime()
    {
        shutdown();
    }

    bool SdlImGuiRuntime::initialize()
    {
        if (sdl_initialized_)
        {
            return true;
        }

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            return false;
        }
        sdl_initialized_ = true;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        window_ = SDL_CreateWindow(
            "Regex Workbench",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1440,
            960,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        );
        if (window_ == nullptr)
        {
            shutdown();
            return false;
        }

        gl_context_ = SDL_GL_CreateContext(window_);
        if (gl_context_ == nullptr)
        {
            shutdown();
            return false;
        }
        SDL_GL_MakeCurrent(window_, gl_context_);
        SDL_GL_SetSwapInterval(1);
        SDL_StartTextInput();
        text_input_started_ = true;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        imgui_context_created_ = true;

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        configure_fonts(io);

        sdl_backend_initialized_ = ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
        if (!sdl_backend_initialized_)
        {
            shutdown();
            return false;
        }
        opengl_backend_initialized_ = ImGui_ImplOpenGL3_Init("#version 100");
        if (!opengl_backend_initialized_)
        {
            shutdown();
            return false;
        }

        return true;
    }

    void SdlImGuiRuntime::shutdown()
    {
        if (opengl_backend_initialized_)
        {
            ImGui_ImplOpenGL3_Shutdown();
            opengl_backend_initialized_ = false;
        }
        if (sdl_backend_initialized_)
        {
            ImGui_ImplSDL2_Shutdown();
            sdl_backend_initialized_ = false;
        }
        if (imgui_context_created_)
        {
            ImGui::DestroyContext();
            imgui_context_created_ = false;
        }
        if (text_input_started_)
        {
            SDL_StopTextInput();
            text_input_started_ = false;
        }
        if (gl_context_ != nullptr)
        {
            SDL_GL_DeleteContext(gl_context_);
            gl_context_ = nullptr;
        }
        if (window_ != nullptr)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        if (sdl_initialized_)
        {
            SDL_Quit();
            sdl_initialized_ = false;
        }
    }

    bool SdlImGuiRuntime::process_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT ||
                (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID(window_)))
            {
                return false;
            }
        }
        return true;
    }

    void SdlImGuiRuntime::begin_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void SdlImGuiRuntime::end_frame()
    {
        ImGui::Render();

        int display_width = 0;
        int display_height = 0;
        SDL_GL_GetDrawableSize(window_, &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);
        const ImVec4 background = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        glClearColor(background.x, background.y, background.z, background.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window_);
    }
}
