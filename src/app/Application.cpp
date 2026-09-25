// Implements top-level application initialization and frame dispatch.
#include "app/Application.hpp"

#include "app/platform/PlatformIntegration.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace app
{
    Application::Application() : main_window_(operation_controller_)
    {
    }

    Application::~Application()
    {
        shutdown();
    }

    bool Application::initialize()
    {
        if (!runtime_.initialize())
        {
            return false;
        }

        platform::install_platform_integration();
        running_ = true;
        return true;
    }

    void Application::run()
    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(
            [](void* context) { static_cast<Application*>(context)->frame(); }, this, 0, true
        );
#else
        while (running_)
        {
            frame();
        }
#endif
    }

    void Application::frame()
    {
        if (!runtime_.process_events())
        {
            running_ = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return;
        }

        runtime_.begin_frame();
        platform::begin_platform_frame();
        main_window_.render();
        runtime_.end_frame();
    }

    void Application::shutdown()
    {
        operation_controller_.cancel();
        platform::reset_platform_integration();
        runtime_.shutdown();
        running_ = false;
    }
}
