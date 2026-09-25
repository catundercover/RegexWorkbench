// Implements host clipboard and browser text-input integration.
#include "app/platform/PlatformIntegration.hpp"

#include <SDL.h>
#include <imgui.h>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

extern "C" void regex_browser_install_handlers();
extern "C" void regex_browser_write_clipboard(const char* text);
extern "C" void regex_browser_clear_clipboard_target();
extern "C" void regex_browser_set_text_input_active(int active);
extern "C" void regex_browser_register_clipboard_target(
    float minimum_x, float minimum_y, float maximum_x, float maximum_y, const char* text
);
#endif

namespace
{
    // Owns clipboard text long enough for ImGui's borrowed return pointer.
    std::string clipboard_cache;
    // Defers browser paste until shortcut modifiers no longer suppress character input.
    std::string pending_paste_text;

    // Returns clipboard text through ImGui's callback interface.
    const char* read_clipboard(void*)
    {
#ifdef __EMSCRIPTEN__
        return clipboard_cache.c_str();
#else
        char* text = SDL_GetClipboardText();
        clipboard_cache = text != nullptr ? text : "";
        SDL_free(text);
        return clipboard_cache.c_str();
#endif
    }

    // Copies ImGui text to the platform clipboard and the stable local cache.
    void write_clipboard(void*, const char* text)
    {
        clipboard_cache = text != nullptr ? text : "";
#ifdef __EMSCRIPTEN__
        regex_browser_write_clipboard(clipboard_cache.c_str());
#else
        SDL_SetClipboardText(clipboard_cache.c_str());
#endif
    }
}

#ifdef __EMSCRIPTEN__
// Forwards browser-generated text input to Dear ImGui.
extern "C" EMSCRIPTEN_KEEPALIVE void regex_browser_text_input(const char* text)
{
    if (text != nullptr)
    {
        ImGui::GetIO().AddInputCharactersUTF8(text);
    }
}

// Queues browser clipboard text until Ctrl/Command is released.
extern "C" EMSCRIPTEN_KEEPALIVE void regex_browser_paste_text(const char* text)
{
    if (text != nullptr)
    {
        clipboard_cache = text;
        pending_paste_text += text;
    }
}
#endif

namespace app::platform
{
    void install_platform_integration()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.SetClipboardTextFn = write_clipboard;
        io.GetClipboardTextFn = read_clipboard;
        io.ClipboardUserData = nullptr;
#ifdef __EMSCRIPTEN__
        regex_browser_install_handlers();
#endif
    }

    void begin_platform_frame()
    {
#ifdef __EMSCRIPTEN__
        regex_browser_clear_clipboard_target();
        ImGuiIO& io = ImGui::GetIO();
        regex_browser_set_text_input_active(io.WantTextInput ? 1 : 0);
        if (!pending_paste_text.empty() && !io.KeyCtrl && !io.KeySuper)
        {
            io.AddInputCharactersUTF8(pending_paste_text.c_str());
            pending_paste_text.clear();
        }
#endif
    }

    void register_browser_clipboard_target(
        const float minimum_x,
        const float minimum_y,
        const float maximum_x,
        const float maximum_y,
        const char* text
    )
    {
#ifdef __EMSCRIPTEN__
        regex_browser_register_clipboard_target(
            minimum_x, minimum_y, maximum_x, maximum_y, text != nullptr ? text : ""
        );
#else
        static_cast<void>(minimum_x);
        static_cast<void>(minimum_y);
        static_cast<void>(maximum_x);
        static_cast<void>(maximum_y);
        static_cast<void>(text);
#endif
    }

    void reset_platform_integration()
    {
        clipboard_cache.clear();
        pending_paste_text.clear();
    }
}
