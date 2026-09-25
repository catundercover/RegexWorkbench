# Defines shared compiler options and optional project quality instrumentation.
include_guard(GLOBAL)

option(REGEXTHESIS_WARNINGS_AS_ERRORS "Treat project warnings as errors" OFF)
option(REGEXTHESIS_ENABLE_CLANG_TIDY "Run clang-tidy while compiling project targets" OFF)
option(REGEXTHESIS_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(REGEXTHESIS_ENABLE_COVERAGE "Instrument native targets for coverage reporting" OFF)

if (REGEXTHESIS_ENABLE_SANITIZERS AND REGEXTHESIS_ENABLE_COVERAGE)
    message(FATAL_ERROR "Sanitizers and coverage instrumentation require separate build trees.")
endif()

add_library(regex_project_options INTERFACE)
add_library(regex_project_warnings INTERFACE)

target_compile_features(regex_project_options INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(regex_project_warnings INTERFACE
            /W4
            /permissive-
            $<$<BOOL:${REGEXTHESIS_WARNINGS_AS_ERRORS}>:/WX>
    )
else()
    target_compile_options(regex_project_warnings INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wformat=2
            -Wundef
            -Wnon-virtual-dtor
            -Woverloaded-virtual
            $<$<BOOL:${REGEXTHESIS_WARNINGS_AS_ERRORS}>:-Werror>
    )
endif()

if (REGEXTHESIS_ENABLE_SANITIZERS)
    if (EMSCRIPTEN)
        message(FATAL_ERROR "REGEXTHESIS_ENABLE_SANITIZERS is supported by native builds only.")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(regex_project_options INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        target_link_options(regex_project_options INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
    else()
        message(FATAL_ERROR "The selected compiler is not configured for project sanitizers.")
    endif()
endif()

if (REGEXTHESIS_ENABLE_COVERAGE)
    if (EMSCRIPTEN)
        message(FATAL_ERROR "REGEXTHESIS_ENABLE_COVERAGE is supported by native builds only.")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(regex_project_options INTERFACE -O0 -g --coverage)
        target_link_options(regex_project_options INTERFACE --coverage)
    else()
        message(FATAL_ERROR "The selected compiler is not configured for coverage reporting.")
    endif()
endif()

if (REGEXTHESIS_ENABLE_CLANG_TIDY)
    if (EMSCRIPTEN)
        message(FATAL_ERROR "Run clang-tidy from a native build tree.")
    endif()
    find_program(REGEXTHESIS_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

# Applies the common language, warning, and optional lint settings to one target.
function(regexthesis_configure_target target)
    cmake_parse_arguments(ARG "NO_LINT" "" "" ${ARGN})

    target_link_libraries(${target} PRIVATE regex_project_options regex_project_warnings)

    if (REGEXTHESIS_ENABLE_CLANG_TIDY AND NOT ARG_NO_LINT)
        set_property(TARGET ${target} PROPERTY CXX_CLANG_TIDY
                "${REGEXTHESIS_CLANG_TIDY_EXECUTABLE};--quiet"
        )
    endif()
endfunction()
