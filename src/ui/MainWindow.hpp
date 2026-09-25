/// @file
/// Declares the application's top-level Dear ImGui window.
#pragma once

#include "ui/main_window/MainWindowState.hpp"
#include "ui/operations/OperationCoordinator.hpp"

#include <optional>

namespace app
{
    namespace operations
    {
        class OperationController;
    }
}

namespace ui
{
    /// Composes the application's top-level UI from its persistent state.
    class MainWindow
    {
    public:
        /// Connects the window state to the shared operation controller.
        explicit MainWindow(app::operations::OperationController& operation_controller);

        /// Renders the full application UI for one frame.
        void render();

    private:
        app::operations::OperationController& operation_controller_;
        operations::OperationCoordinator operation_coordinator_;
        MainWindowState state_;
        std::optional<app::settings::ColorTheme> applied_color_theme_;
    };
}
