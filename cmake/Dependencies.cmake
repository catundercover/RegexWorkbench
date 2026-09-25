# Reports project dependencies that must be present in the source tree.
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp")
    message(WARNING "Dear ImGui was not found in external/imgui.")
    message(WARNING "Clone it with:")
    message(WARNING "git submodule add https://github.com/ocornut/imgui external/imgui")
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/mata")
    message(WARNING "mata was not found in external/mata.")
    message(WARNING "Add mata as submodule or copy the library there.")
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot")
    message(WARNING "Spot was not found in external/spot.")
    message(WARNING "Add Spot as submodule or copy the library there.")
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/source")
    message(WARNING "Spot source was not found in external/spot/source.")
    message(WARNING "Recommended layout:")
    message(WARNING "  external/spot/source")
    message(WARNING "  external/spot/native")
    message(WARNING "  external/spot/wasm")
endif()

if (EMSCRIPTEN)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/wasm/include/spot/twa/twagraph.hh")
        message(WARNING "Emscripten-built Spot headers were not found in external/spot/wasm/include.")
    endif()

    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/wasm/lib/libspot.a")
        message(WARNING "Emscripten-built libspot.a was not found in external/spot/wasm/lib.")
    endif()
else()
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/native/include/spot/twa/twagraph.hh")
        message(WARNING "Native Spot headers were not found in external/spot/native/include.")
    endif()

    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/native/lib/libspot.a" AND
            NOT EXISTS "${CMAKE_SOURCE_DIR}/external/spot/native/lib/libspot.so")
        message(WARNING "Native libspot was not found in external/spot/native/lib.")
    endif()
endif()