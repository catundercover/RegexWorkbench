# Configures native and WebAssembly application, worker, test
cmake_minimum_required(VERSION 3.20)

project(RegexThesis
        VERSION 1.0.0
        LANGUAGES CXX
)

include(CTest)
include(cmake/ProjectOptions.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT "" CACHE PATH
        "Path to an Emscripten-built Graphviz installation prefix")

set(REGEXTHESIS_SPOT_ROOT "" CACHE PATH
        "Path to a project-local Spot installation prefix")

include(cmake/Dependencies.cmake)
include(FetchContent)

if (REGEXTHESIS_SPOT_ROOT STREQUAL "")
    if (EMSCRIPTEN)
        set(REGEXTHESIS_SPOT_ROOT "${CMAKE_SOURCE_DIR}/external/spot/wasm")
    else()
        set(REGEXTHESIS_SPOT_ROOT "${CMAKE_SOURCE_DIR}/external/spot/native")
    endif()
endif()

set(REGEXTHESIS_SPOT_INCLUDE_DIR "${REGEXTHESIS_SPOT_ROOT}/include")
set(REGEXTHESIS_SPOT_LIB_DIR "${REGEXTHESIS_SPOT_ROOT}/lib")

find_path(REGEXTHESIS_SPOT_INCLUDE
        NAMES spot/twa/twagraph.hh
        PATHS "${REGEXTHESIS_SPOT_INCLUDE_DIR}"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
)

find_file(REGEXTHESIS_SPOT_LIBRARY
        NAMES libspot.a libspot.so
        PATHS "${REGEXTHESIS_SPOT_LIB_DIR}"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
)

find_file(REGEXTHESIS_BDDX_LIBRARY
        NAMES libbddx.a libbddx.so
        PATHS "${REGEXTHESIS_SPOT_LIB_DIR}"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
)

if (NOT REGEXTHESIS_SPOT_INCLUDE OR NOT REGEXTHESIS_SPOT_LIBRARY)
    message(FATAL_ERROR
            "Project-local Spot was not found.\n"
            "Expected Spot under:\n"
            "  ${REGEXTHESIS_SPOT_ROOT}\n"
            "\n"
            "Expected files:\n"
            "  ${REGEXTHESIS_SPOT_ROOT}/include/spot/twa/twagraph.hh\n"
            "  ${REGEXTHESIS_SPOT_ROOT}/lib/libspot.a or libspot.so\n"
            "\n"
            "For native builds, build/install Spot into:\n"
            "  external/spot/native\n"
            "\n"
            "For WebAssembly builds, build/install Spot with Emscripten into:\n"
            "  external/spot/wasm\n"
            "\n"
            "You may also pass:\n"
            "  -DREGEXTHESIS_SPOT_ROOT=/path/to/spot/install"
    )
endif()

if (NOT REGEXTHESIS_BDDX_LIBRARY)
    message(FATAL_ERROR
            "Project-local Spot was found, but libbddx was not found.\n"
            "Expected:\n"
            "  ${REGEXTHESIS_SPOT_LIB_DIR}/libbddx.a or libbddx.so"
    )
endif()

add_library(regex_spot INTERFACE)

target_include_directories(regex_spot SYSTEM INTERFACE
        "${REGEXTHESIS_SPOT_INCLUDE}"
)

target_link_libraries(regex_spot INTERFACE
        "${REGEXTHESIS_SPOT_LIBRARY}"
        "${REGEXTHESIS_BDDX_LIBRARY}"
)

message(STATUS "Using project-local Spot from: ${REGEXTHESIS_SPOT_ROOT}")

FetchContent_Declare(
        lexy
        GIT_REPOSITORY https://github.com/foonathan/lexy.git
        GIT_TAG        c1358c4117752393e9ce27d6c885d9146846f2f9
)

set(LEXY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(LEXY_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(lexy)

if (NOT EMSCRIPTEN)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(OpenGL REQUIRED COMPONENTS GLES2)
endif()

set(REGEXTHESIS_COMMON_INCLUDE_DIRS
        src
)

set(REGEXTHESIS_MATA_INCLUDE_DIRS
        external/mata/include
        external/mata/3rdparty/simlib/include
)

set(REGEXTHESIS_IMGUI_INCLUDE_DIRS
        external/imgui
        external/imgui/backends
        external/imgui/misc/cpp
)

add_library(regex_imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/imgui/backends/imgui_impl_sdl2.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(regex_imgui PUBLIC
        ${REGEXTHESIS_IMGUI_INCLUDE_DIRS}
)

if (EMSCRIPTEN)
    target_compile_options(regex_imgui PRIVATE -sUSE_SDL=2)
else()
    target_link_libraries(regex_imgui PRIVATE SDL2::SDL2 OpenGL::GLES2)
endif()

add_library(regex_core STATIC
        src/regex/model/Expression.cpp
        src/regex/model/Expression.hpp
        src/regex/model/OmegaExpression.cpp
        src/regex/model/OmegaExpression.hpp
        src/regex/inspection/ExpressionInspection.cpp
        src/regex/inspection/ExpressionInspection.hpp
        src/regex/inspection/OmegaExpressionInspection.cpp
        src/regex/inspection/OmegaExpressionInspection.hpp
        src/regex/formatting/RegexFormatter.cpp
        src/regex/formatting/RegexFormatter.hpp
        src/regex/formatting/OmegaRegexFormatter.cpp
        src/regex/formatting/OmegaRegexFormatter.hpp
        src/regex/simplification/RegexSimplifier.cpp
        src/regex/simplification/RegexSimplifier.hpp
        src/regex/simplification/OmegaRegexSimplifier.cpp
        src/regex/simplification/OmegaRegexSimplifier.hpp
)

target_include_directories(regex_core PUBLIC
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

add_library(regex_parser STATIC
        src/regex/parsing/InputNormalization.cpp
        src/regex/parsing/InputNormalization.hpp
        src/regex/parsing/ParsingDiagnostics.cpp
        src/regex/parsing/ParsingDiagnostics.hpp
        src/regex/parsing/RegexGrammar.hpp
        src/regex/parsing/RegexParser.cpp
        src/regex/parsing/RegexParser.hpp
        src/regex/parsing/OmegaRegexParser.cpp
        src/regex/parsing/OmegaRegexParser.hpp
)

target_include_directories(regex_parser PUBLIC
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

target_link_libraries(regex_parser
        PUBLIC regex_core
        PRIVATE
        foonathan::lexy
        foonathan::lexy::ext
)

# GCC 16 reports false maybe-uninitialized diagnostics while inlining lexy's
# lazy_init<std::variant<...>> implementation in optimized builds.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT REGEXTHESIS_ENABLE_CLANG_TIDY)
    target_compile_options(regex_parser PRIVATE -Wno-maybe-uninitialized)
endif()

add_library(regex_generator STATIC
        src/regex/generation/RandomRegexGenerator.cpp
        src/regex/generation/RandomRegexGenerator.hpp
        src/regex/generation/RandomOmegaRegexGenerator.cpp
        src/regex/generation/RandomOmegaRegexGenerator.hpp
)

target_include_directories(regex_generator PUBLIC
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

target_link_libraries(regex_generator PUBLIC
        regex_core
)

add_library(regex_app_settings STATIC
        src/app/settings/ApplicationSettings.cpp
        src/app/settings/ApplicationSettings.hpp
)

target_include_directories(regex_app_settings PUBLIC src)

target_link_libraries(regex_app_settings PUBLIC regex_generator)

add_library(regex_mata STATIC
        external/mata/src/nfa/delta.cc
        external/mata/src/nfa/nfa.cc
        external/mata/src/nfa/operations.cc
        external/mata/src/nfa/inclusion.cc
        external/mata/src/nfa/product.cc
        external/mata/src/nfa/complement.cc
        external/mata/src/alphabet.cc
        external/mata/3rdparty/simlib/src/explicit_lts_sim.cc
)

target_include_directories(regex_mata SYSTEM PUBLIC
        ${REGEXTHESIS_MATA_INCLUDE_DIRS}
)

add_library(regex_automata STATIC
        src/automata/model/Nfa.cpp
        src/automata/model/Nfa.hpp
        src/automata/model/BuchiAutomaton.hpp

        src/automata/conversion/MataAdapter.cpp
        src/automata/conversion/MataAdapter.hpp
        src/automata/conversion/NfaToRegex.cpp
        src/automata/conversion/NfaToRegex.hpp
        src/automata/conversion/RegexAlphabet.cpp
        src/automata/conversion/RegexAlphabet.hpp
        src/automata/conversion/RegexToNfa.cpp
        src/automata/conversion/RegexToNfa.hpp
        src/automata/conversion/OmegaRegexAlphabet.cpp
        src/automata/conversion/OmegaRegexAlphabet.hpp
        src/automata/conversion/SpotAlphabetEncoding.cpp
        src/automata/conversion/SpotAlphabetEncoding.hpp
        src/automata/conversion/SpotOmegaAdapter.cpp
        src/automata/conversion/SpotOmegaAdapter.hpp
        src/automata/conversion/OmegaRegexToSpot.cpp
        src/automata/conversion/OmegaRegexToSpot.hpp
        src/automata/conversion/SpotToOmegaRegex.cpp
        src/automata/conversion/SpotToOmegaRegex.hpp

        src/automata/translation/RegexTranslation.cpp
        src/automata/translation/RegexTranslation.hpp
        src/automata/translation/OmegaRegexTranslation.cpp
        src/automata/translation/OmegaRegexTranslation.hpp

        src/automata/dot/DotExporter.cpp
        src/automata/dot/DotExporter.hpp
        src/automata/dot/BuchiDotExporter.cpp
        src/automata/dot/BuchiDotExporter.hpp

        src/automata/analysis/RegexComparison.cpp
        src/automata/analysis/RegexComparison.hpp
        src/automata/analysis/OmegaRegexComparison.cpp
        src/automata/analysis/OmegaRegexComparison.hpp

        src/automata/rewrite/RegexRewriter.cpp
        src/automata/rewrite/RegexRewriter.hpp
        src/automata/rewrite/OmegaRegexRewriter.cpp
        src/automata/rewrite/OmegaRegexRewriter.hpp
)

target_include_directories(regex_automata PUBLIC
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

target_link_libraries(regex_automata
        PUBLIC
        regex_core
        regex_spot
        PRIVATE
        regex_mata
        regex_parser
)

add_library(regex_graph_model INTERFACE)

target_include_directories(regex_graph_model INTERFACE
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

add_library(regex_graphviz_layout STATIC
        src/graph/model/Geometry.hpp
        src/graph/model/Layout.hpp
        src/graph/graphviz/GraphvizLayout.cpp
        src/graph/graphviz/GraphvizLayout.hpp
        src/graph/graphviz/detail/AttributeParser.cpp
        src/graph/graphviz/detail/AttributeParser.hpp
        src/graph/graphviz/detail/LayoutExtractor.cpp
        src/graph/graphviz/detail/LayoutExtractor.hpp
)

target_include_directories(regex_graphviz_layout PRIVATE
        src
)

target_link_libraries(regex_graphviz_layout PUBLIC
        regex_graph_model
)

if (EMSCRIPTEN)
    if (NOT REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT)
        set(REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT "${CMAKE_SOURCE_DIR}/external/graphviz-wasm")
    endif()
    set(REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR
            "${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT}/lib")

    target_include_directories(regex_graphviz_layout SYSTEM PRIVATE
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT}/include
    )

    target_compile_definitions(regex_graphviz_layout PRIVATE
            REGEXTHESIS_STATIC_GRAPHVIZ_PLUGINS=1
    )

    target_link_libraries(regex_graphviz_layout PRIVATE
            -Wl,--start-group
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libgvc.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libcgraph.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libcdt.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libpathplan.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libgvplugin_dot_layout.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libgvplugin_core.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libxdot.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libdotgen.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libfdpgen.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libsfdpgen.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libcircogen.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libtwopigen.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libosage.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libpatchwork.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libvpsc.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libsparse.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/librbtree.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libcommon.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libpack.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/liblabel.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libutil.a
            ${REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_LIB_DIR}/libortho.a
            -Wl,--end-group
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRAPHVIZ REQUIRED IMPORTED_TARGET libgvc libcgraph libcdt)

    target_link_libraries(regex_graphviz_layout PRIVATE
            PkgConfig::GRAPHVIZ
    )
endif()

add_library(regex_operation_models INTERFACE)

target_include_directories(regex_operation_models INTERFACE src)

target_link_libraries(regex_operation_models INTERFACE
        regex_graph_model
        regex_automata)

add_library(regex_operation_core STATIC
        src/app/operations/ExpressionFlavor.hpp
        src/app/operations/OperationExecutor.cpp
        src/app/operations/OperationExecutor.hpp
        src/app/operations/OperationLimits.hpp
        src/app/operations/OperationModels.hpp
        src/app/operations/OperationValidation.cpp
        src/app/operations/OperationValidation.hpp
)

target_include_directories(regex_operation_core PUBLIC
        src
)

target_link_libraries(regex_operation_core PUBLIC
        regex_operation_models
        regex_automata
        regex_graph_model
)

target_link_libraries(regex_operation_core PRIVATE
        regex_graphviz_layout
        regex_parser
)

if (EMSCRIPTEN)
    target_compile_options(regex_operation_core PRIVATE
            -sNO_DISABLE_EXCEPTION_CATCHING
    )
    target_link_options(regex_operation_core INTERFACE
            -sNO_DISABLE_EXCEPTION_CATCHING
    )
endif()

add_library(regex_operation_protocol STATIC
        src/worker/protocol/OperationProtocol.cpp
        src/worker/protocol/OperationProtocol.hpp
        src/worker/protocol/PayloadFrame.cpp
        src/worker/protocol/PayloadFrame.hpp
        src/worker/protocol/ProtocolLimits.hpp
        src/worker/protocol/detail/BinaryCodec.cpp
        src/worker/protocol/detail/BinaryCodec.hpp
        src/worker/protocol/detail/GraphCodec.cpp
        src/worker/protocol/detail/GraphCodec.hpp
)

target_include_directories(regex_operation_protocol PUBLIC
        src
)

target_link_libraries(regex_operation_protocol PUBLIC regex_operation_models)

add_executable(RegexOperationWorker
        src/worker/OperationWorker.cpp
        src/worker/OperationWorker.hpp
)

target_include_directories(RegexOperationWorker PRIVATE
        src
)

target_link_libraries(RegexOperationWorker PRIVATE
        regex_operation_core
        regex_operation_protocol
)

add_library(regex_app_operations STATIC
        src/app/operations/OperationController.cpp
        src/app/operations/OperationController.hpp
        src/app/operations/OperationRunner.hpp
        src/app/operations/PlatformOperationRunner.cpp
        src/app/operations/detail/PlatformRunnerFactory.hpp
)

target_include_directories(regex_app_operations PUBLIC
        src
)

target_link_libraries(regex_app_operations
        PUBLIC regex_operation_core
        PRIVATE regex_operation_protocol
)

if (EMSCRIPTEN)
    target_sources(RegexOperationWorker PRIVATE
            src/worker/entry/WebWorkerEntry.cpp
    )
    target_sources(regex_app_operations PRIVATE
            src/app/operations/detail/WebOperationRunner.cpp
    )
    target_compile_options(regex_operation_protocol PRIVATE
            -sNO_DISABLE_EXCEPTION_CATCHING
    )
    set_property(TARGET RegexOperationWorker PROPERTY OUTPUT_NAME "operation-worker")
    set_property(TARGET RegexOperationWorker PROPERTY SUFFIX ".js")
    target_compile_options(RegexOperationWorker PRIVATE
            -sNO_DISABLE_EXCEPTION_CATCHING
    )
    target_link_options(RegexOperationWorker PRIVATE
            -sBUILD_AS_WORKER=1
            -sALLOW_MEMORY_GROWTH=1
            -sNO_DISABLE_EXCEPTION_CATCHING
            -sEXPORTED_FUNCTIONS=['_run_operation']
    )
    target_compile_options(regex_app_operations PRIVATE
            -sNO_DISABLE_EXCEPTION_CATCHING
    )
else()
    target_sources(RegexOperationWorker PRIVATE
            src/worker/entry/NativeWorkerMain.cpp
    )
    target_sources(regex_app_operations PRIVATE
            src/app/operations/detail/NativeOperationRunner.cpp
    )
    target_compile_definitions(regex_app_operations PRIVATE
            REGEXTHESIS_OPERATION_WORKER_NAME="RegexOperationWorker"
    )
endif()

add_library(regex_ui_input_logic STATIC
        src/ui/input/ExpressionInputValidation.cpp
        src/ui/input/ExpressionInputValidation.hpp
        src/ui/input/MacroCompletionLogic.cpp
        src/ui/input/MacroCompletionLogic.hpp
        src/ui/input/RandomRegexLogic.cpp
        src/ui/input/RandomRegexLogic.hpp
)

target_include_directories(regex_ui_input_logic PUBLIC
        src
)

target_link_libraries(regex_ui_input_logic PUBLIC
        regex_parser
)

add_library(regex_ui_graph_model STATIC
        src/ui/graph/AutomatonText.cpp
        src/ui/graph/AutomatonText.hpp
        src/ui/graph/GraphCanvasModel.cpp
        src/ui/graph/GraphCanvasModel.hpp
)

target_include_directories(regex_ui_graph_model PUBLIC
        src
)

target_link_libraries(regex_ui_graph_model PUBLIC
        regex_graph_model
)

add_library(regex_ui_comparison_model STATIC
        src/ui/comparison/ComparisonModel.cpp
        src/ui/comparison/ComparisonModel.hpp
)

target_include_directories(regex_ui_comparison_model PUBLIC
        src
)

target_link_libraries(regex_ui_comparison_model PUBLIC
        regex_automata
)

add_library(regex_ui_operations STATIC
        src/ui/operations/OperationCoordinator.cpp
        src/ui/operations/OperationCoordinator.hpp
        src/ui/operations/OperationState.hpp
)

target_include_directories(regex_ui_operations PUBLIC
        src
)

target_link_libraries(regex_ui_operations PUBLIC
        regex_app_operations
        regex_ui_comparison_model
        regex_ui_graph_model
)

target_link_libraries(regex_ui_operations PRIVATE
        regex_parser
)

add_library(regex_ui STATIC
        src/ui/MainWindow.cpp
        src/ui/MainWindow.hpp
        src/ui/components/UiComponents.cpp
        src/ui/components/UiComponents.hpp
        src/ui/comparison/ComparisonDiagram.cpp
        src/ui/comparison/ComparisonDiagram.hpp
        src/ui/graph/AutomatonGraphCanvas.cpp
        src/ui/graph/AutomatonGraphCanvas.hpp
        src/ui/input/AlphabetInput.cpp
        src/ui/input/AlphabetInput.hpp
        src/ui/input/MacroCompletion.cpp
        src/ui/input/MacroCompletion.hpp
        src/ui/input/RandomRegexControls.cpp
        src/ui/input/RandomRegexControls.hpp
        src/ui/input/RegexInput.cpp
        src/ui/input/RegexInput.hpp
        src/ui/main_window/MainWindowState.hpp
        src/ui/operations/OperationPanel.cpp
        src/ui/operations/OperationPanel.hpp
        src/ui/results/ComparisonResultView.cpp
        src/ui/results/ComparisonResultView.hpp
        src/ui/results/GraphResultView.cpp
        src/ui/results/GraphResultView.hpp
        src/ui/results/OperationOutputView.cpp
        src/ui/results/OperationOutputView.hpp
        src/ui/results/RewriteResultView.cpp
        src/ui/results/RewriteResultView.hpp
        src/ui/results/ResultAction.hpp
        src/ui/results/ResultWidgets.cpp
        src/ui/results/ResultWidgets.hpp
        src/ui/settings/SettingsPage.cpp
        src/ui/settings/SettingsPage.hpp
        src/ui/style/ApplicationTheme.cpp
        src/ui/style/ApplicationTheme.hpp
)

target_include_directories(regex_ui PUBLIC src)

target_include_directories(regex_ui SYSTEM PUBLIC
        ${REGEXTHESIS_IMGUI_INCLUDE_DIRS}
)

target_link_libraries(regex_ui PUBLIC
        regex_app_settings
        regex_app_operations
        regex_generator
        regex_ui_comparison_model
        regex_ui_graph_model
        regex_ui_input_logic
        regex_ui_operations
)

add_executable(RegexThesis
        src/main.cpp

        src/app/Application.cpp
        src/app/Application.hpp
        src/app/platform/PlatformIntegration.cpp
        src/app/platform/PlatformIntegration.hpp
        src/app/runtime/SdlImGuiRuntime.cpp
        src/app/runtime/SdlImGuiRuntime.hpp
)

target_include_directories(RegexThesis PRIVATE
        src
        ${REGEXTHESIS_COMMON_INCLUDE_DIRS}
)

target_include_directories(RegexThesis SYSTEM PRIVATE
        ${REGEXTHESIS_IMGUI_INCLUDE_DIRS}
)

target_link_libraries(RegexThesis PRIVATE
        regex_app_operations
        regex_ui
        regex_imgui
)

foreach(target IN ITEMS
        regex_core
        regex_parser
        regex_generator
        regex_app_settings
        regex_automata
        regex_graphviz_layout
        regex_operation_core
        regex_operation_protocol
        RegexOperationWorker
        regex_app_operations
        regex_ui_input_logic
        regex_ui_graph_model
        regex_ui_comparison_model
        regex_ui_operations
        regex_ui
        RegexThesis
)
    regexthesis_configure_target(${target})
endforeach()

add_dependencies(RegexThesis RegexOperationWorker)

if (NOT EMSCRIPTEN)
    target_link_libraries(RegexThesis PRIVATE SDL2::SDL2 OpenGL::GLES2)

    add_custom_command(TARGET RegexThesis POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/external/imgui/fonts
                    $<TARGET_FILE_DIR:RegexThesis>/fonts
    )
endif()

if (BUILD_TESTING)
    add_executable(regex_omega_regex_language_test
            tests/OmegaRegexLanguageTest.cpp
    )

    target_include_directories(regex_omega_regex_language_test PRIVATE tests)

    target_link_libraries(regex_omega_regex_language_test PRIVATE
            regex_automata
            regex_parser
    )

    add_executable(regex_omega_automata_algorithms_test
            tests/OmegaAutomataAlgorithmsTest.cpp
    )

    target_include_directories(regex_omega_automata_algorithms_test PRIVATE tests)

    target_link_libraries(regex_omega_automata_algorithms_test PRIVATE
            regex_operation_core
            regex_automata
            regex_parser
    )

    add_executable(regex_omega_regex_rewrite_test
            tests/OmegaRegexRewriteTest.cpp
    )

    target_include_directories(regex_omega_regex_rewrite_test PRIVATE tests)

    target_link_libraries(regex_omega_regex_rewrite_test PRIVATE
            regex_operation_core
            regex_automata
            regex_parser
    )

    add_executable(regex_automata_model_test
            tests/AutomataModelTest.cpp
    )

    target_link_libraries(regex_automata_model_test PRIVATE
            regex_automata
    )

    add_executable(regex_automata_algorithms_test
            tests/AutomataAlgorithmsTest.cpp
    )

    target_link_libraries(regex_automata_algorithms_test PRIVATE
            regex_automata
            regex_parser
    )

    add_executable(regex_language_test
            tests/RegexLanguageTest.cpp
    )

    target_link_libraries(regex_language_test PRIVATE
            regex_generator
            regex_parser
    )

    add_executable(regex_properties_test
            tests/RegexPropertiesTest.cpp
    )

    target_include_directories(regex_properties_test PRIVATE tests)

    target_link_libraries(regex_properties_test PRIVATE
            regex_generator
            regex_parser
    )

    add_executable(regex_automata_properties_test
            tests/AutomataPropertiesTest.cpp
    )

    target_include_directories(regex_automata_properties_test PRIVATE tests)

    target_link_libraries(regex_automata_properties_test PRIVATE
            regex_automata
            regex_generator
            regex_parser
    )

    add_executable(regex_app_operations_test
            tests/OperationControllerTest.cpp
    )

    target_include_directories(regex_app_operations_test PRIVATE tests)

    target_link_libraries(regex_app_operations_test PRIVATE
            regex_app_operations
    )

    if (NOT EMSCRIPTEN)
        add_dependencies(regex_app_operations_test RegexOperationWorker)
    endif()

    add_executable(regex_app_settings_test
            tests/ApplicationSettingsTest.cpp
    )

    target_link_libraries(regex_app_settings_test PRIVATE regex_app_settings)

    add_executable(regex_operation_protocol_test
            tests/OperationProtocolTest.cpp
    )

    target_link_libraries(regex_operation_protocol_test PRIVATE
            regex_operation_protocol
    )

    add_executable(regex_operation_protocol_robustness_test
            tests/OperationProtocolRobustnessTest.cpp
    )

    target_include_directories(regex_operation_protocol_robustness_test PRIVATE tests)

    target_link_libraries(regex_operation_protocol_robustness_test PRIVATE
            regex_operation_protocol
            regex_parser
    )

    add_executable(regex_operation_limits_test
            tests/OperationLimitsTest.cpp
    )

    target_include_directories(regex_operation_limits_test PRIVATE tests)

    target_link_libraries(regex_operation_limits_test PRIVATE
            regex_operation_core
            regex_parser
    )

    add_executable(regex_graph_layout_test
            tests/GraphLayoutTest.cpp
    )

    target_link_libraries(regex_graph_layout_test PRIVATE
            regex_graphviz_layout
    )

    add_executable(regex_ui_graph_model_test
            tests/GraphCanvasModelTest.cpp
    )

    target_include_directories(regex_ui_graph_model_test PRIVATE tests)

    target_link_libraries(regex_ui_graph_model_test PRIVATE
            regex_ui_graph_model
    )

    add_executable(regex_ui_input_logic_test
            tests/UiInputLogicTest.cpp
    )

    target_link_libraries(regex_ui_input_logic_test PRIVATE
            regex_operation_core
            regex_ui_input_logic
    )

    add_executable(regex_ui_comparison_model_test
            tests/ComparisonModelTest.cpp
    )

    target_link_libraries(regex_ui_comparison_model_test PRIVATE
            regex_ui_comparison_model
    )

    add_executable(regex_ui_operations_test
            tests/OperationCoordinatorTest.cpp
    )

    target_include_directories(regex_ui_operations_test PRIVATE tests)

    target_link_libraries(regex_ui_operations_test PRIVATE
            regex_ui_operations
    )

    foreach(test_target IN ITEMS
            regex_automata_model_test
            regex_automata_algorithms_test
            regex_automata_properties_test
            regex_omega_regex_language_test
            regex_omega_automata_algorithms_test
            regex_omega_regex_rewrite_test
            regex_language_test
            regex_properties_test
            regex_app_operations_test
            regex_app_settings_test
            regex_operation_limits_test
            regex_operation_protocol_test
            regex_operation_protocol_robustness_test
            regex_graph_layout_test
            regex_ui_graph_model_test
            regex_ui_input_logic_test
            regex_ui_comparison_model_test
            regex_ui_operations_test
    )
        regexthesis_configure_target(${test_target})
    endforeach()

    if (EMSCRIPTEN)
        foreach(test_target IN ITEMS
                regex_automata_model_test
                regex_automata_algorithms_test
                regex_automata_properties_test
                regex_omega_regex_language_test
                regex_omega_automata_algorithms_test
                regex_omega_regex_rewrite_test
                regex_language_test
                regex_properties_test
                regex_app_operations_test
                regex_app_settings_test
                regex_operation_limits_test
                regex_operation_protocol_test
                regex_operation_protocol_robustness_test
                regex_graph_layout_test
                regex_ui_graph_model_test
                regex_ui_input_logic_test
                regex_ui_comparison_model_test
                regex_ui_operations_test
        )
            set_property(TARGET ${test_target} PROPERTY SUFFIX ".js")
            target_compile_options(${test_target} PRIVATE -sNO_DISABLE_EXCEPTION_CATCHING)
            target_link_options(${test_target} PRIVATE
                    -sALLOW_MEMORY_GROWTH=1
                    -sNO_DISABLE_EXCEPTION_CATCHING
            )
        endforeach()
    endif()

    add_test(
            NAME regex_omega_regex_language
            COMMAND regex_omega_regex_language_test
    )

    add_test(
            NAME regex_omega_automata_algorithms
            COMMAND regex_omega_automata_algorithms_test
    )

    add_test(
            NAME regex_omega_regex_rewrite
            COMMAND regex_omega_regex_rewrite_test
    )

    add_test(
            NAME regex_automata_model
            COMMAND regex_automata_model_test
    )

    add_test(
            NAME regex_automata_algorithms
            COMMAND regex_automata_algorithms_test
    )

    add_test(
            NAME regex_automata_properties
            COMMAND regex_automata_properties_test
    )

    add_test(
            NAME regex_language
            COMMAND regex_language_test
    )

    add_test(
            NAME regex_properties
            COMMAND regex_properties_test
    )

    add_test(
            NAME regex_app_operations
            COMMAND regex_app_operations_test
    )

    add_test(
            NAME regex_app_settings
            COMMAND regex_app_settings_test
    )

    add_test(
            NAME regex_operation_protocol
            COMMAND regex_operation_protocol_test
    )

    add_test(
            NAME regex_operation_protocol_robustness
            COMMAND regex_operation_protocol_robustness_test
    )

    add_test(
            NAME regex_operation_limits
            COMMAND regex_operation_limits_test
    )

    add_test(
            NAME regex_graph_layout
            COMMAND regex_graph_layout_test
    )

    add_test(
            NAME regex_ui_graph_model
            COMMAND regex_ui_graph_model_test
    )

    add_test(
            NAME regex_ui_input_logic
            COMMAND regex_ui_input_logic_test
    )

    add_test(
            NAME regex_ui_comparison_model
            COMMAND regex_ui_comparison_model_test
    )

    add_test(
            NAME regex_ui_operations
            COMMAND regex_ui_operations_test
    )

    if (EMSCRIPTEN)
        find_program(REGEXTHESIS_NODE_EXECUTABLE node REQUIRED)
        add_test(
                NAME regex_browser_bridge
                COMMAND ${REGEXTHESIS_NODE_EXECUTABLE}
                        ${CMAKE_SOURCE_DIR}/tests/BrowserBridgeTest.js
                        ${CMAKE_SOURCE_DIR}/web/browser_bridge.js
        )
    endif()

    if (REGEXTHESIS_ENABLE_SANITIZERS)
        set_tests_properties(
                regex_app_operations
                regex_graph_layout
                regex_omega_automata_algorithms
                regex_ui_operations
                PROPERTIES ENVIRONMENT
                "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/cmake/lsan.supp"
        )
    endif()
endif()

if (EMSCRIPTEN)
    set_property(TARGET RegexThesis PROPERTY OUTPUT_NAME "index")
    set_property(TARGET RegexThesis PROPERTY SUFFIX ".html")
    configure_file(
            ${CMAKE_SOURCE_DIR}/web/style.css
            ${CMAKE_CURRENT_BINARY_DIR}/style.css
            COPYONLY
    )

    target_compile_options(RegexThesis PRIVATE
            -sUSE_SDL=2
    )

    target_link_options(RegexThesis PRIVATE
            -sUSE_SDL=2
            -sWASM=1
            -sALLOW_MEMORY_GROWTH=1
            -sASSERTIONS=1
            -sEXIT_RUNTIME=0
            --shell-file=${CMAKE_SOURCE_DIR}/web/shell.html
            --js-library=${CMAKE_SOURCE_DIR}/web/browser_bridge.js
            --embed-file ${CMAKE_SOURCE_DIR}/external/imgui/fonts@/fonts
            -sEXPORTED_FUNCTIONS=['_main','_malloc','_free','_regex_browser_text_input','_regex_browser_paste_text']
            -sEXPORTED_RUNTIME_METHODS=['UTF8ToString','stringToUTF8','lengthBytesUTF8']
    )

    target_link_options(RegexThesis PRIVATE
            $<$<CONFIG:Debug>:-sASSERTIONS=2>
            $<$<CONFIG:Debug>:-sSAFE_HEAP=1>
            $<$<CONFIG:Debug>:-sSTACK_OVERFLOW_CHECK=2>
            $<$<CONFIG:Debug>:-sRUNTIME_DEBUG=1>
            $<$<CONFIG:Debug>:-gsource-map>
    )

    target_compile_options(RegexThesis PRIVATE
            $<$<CONFIG:Debug>:-gsource-map>
    )

    target_compile_options(RegexThesis PRIVATE
            $<$<CONFIG:Release>:-O2>
            $<$<CONFIG:Release>:-fno-strict-aliasing>
            $<$<CONFIG:Release>:-fno-omit-frame-pointer>
    )
endif()
