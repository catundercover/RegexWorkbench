// Declares the top-level application composition and lifecycle.
#pragma once

#include "app/operations/OperationController.hpp"
#include "app/runtime/SdlImGuiRuntime.hpp"
#include "ui/MainWindow.hpp"

namespace app
{
    // Composes the platform runtime, operation service, and top-level UI.
    class Application
    {
    public:
        // Constructs the application and connects the UI to its operation controller.
        Application();
        // Releases any runtime resources still owned by the application.
        ~Application();

        // Applications own unique platform resources and cannot be copied.
        Application(const Application&) = delete;
        // Applications own unique platform resources and cannot be copied.
        Application& operator=(const Application&) = delete;

        //Initializes the platform runtime and returns whether startup succeeded.
        bool initialize();
        // Runs frames until the platform requests shutdown.
        void run();
        //Releases the UI and platform runtime resources.
        void shutdown();

    private:
        // Renders and presents one application frame.
        void frame();

        runtime::SdlImGuiRuntime runtime_;
        operations::OperationController operation_controller_;
        ui::MainWindow main_window_;
        bool running_ = false;
    };
}
