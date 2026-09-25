// Declares host-specific Dear ImGui integration hooks.
#pragma once

namespace app::platform
{
    // Connects ImGui to the host clipboard and browser-only input behavior.
    void install_platform_integration();
    // Refreshes browser clipboard targets and whether ImGui currently owns text input.
    void begin_platform_frame();
    // Registers a visible copy button so browsers can write during the originating pointer event.
    void register_browser_clipboard_target(
        float minimum_x, float minimum_y, float maximum_x, float maximum_y, const char* text
    );
    // Removes callbacks and transient host integration state.
    void reset_platform_integration();
}
