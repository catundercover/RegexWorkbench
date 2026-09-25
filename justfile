# Defines repeatable configure, build, test, quality, and packaging workflows.
set shell := ["bash", "-euo", "pipefail", "-c"]

project := "RegexThesis"
native_build_dir := "build/native-debug"
native_release_build_dir := "build/native-release"
web_build_dir := "build/web-debug"
web_release_build_dir := "build/web-release"
web_test_build_dir := "build/web-tests"
warnings_build_dir := "build/warnings"
warnings_release_build_dir := "build/warnings-release"
lint_build_dir := "build/lint"
sanitizer_build_dir := "build/sanitizers"
coverage_build_dir := "build/coverage"
web_port := "8080"

# List the available project commands.
default:
    @just --list

# Print the build directories used by the recipes.
info:
    @echo "Project: {{ project }}"
    @echo "Native debug: {{ native_build_dir }}"
    @echo "Native release: {{ native_release_build_dir }}"
    @echo "Web debug: {{ web_build_dir }}"
    @echo "Web release: {{ web_release_build_dir }}"
    @echo "Web tests: {{ web_test_build_dir }}"
    @echo "Web port: {{ web_port }}"

# Check native build prerequisites.
doctor-native:
    @status=0; \
    for tool in cmake ninja c++ pkg-config; do \
        if command -v "$tool" >/dev/null; then \
            echo "$tool: OK"; \
        else \
            echo "$tool: MISSING"; \
            status=1; \
        fi; \
    done; \
    for directory in external/imgui external/mata external/spot external/spot/source; do \
        if [ -d "$directory" ]; then \
            echo "$directory: OK"; \
        else \
            echo "$directory: MISSING"; \
            status=1; \
        fi; \
    done; \
    for path in \
        external/spot/native/include/spot/twa/twagraph.hh \
        external/spot/native/lib/libspot.a \
        external/spot/native/lib/libbddx.a; do \
        if [ -e "$path" ]; then \
            echo "$path: OK"; \
        else \
            echo "$path: MISSING"; \
            status=1; \
        fi; \
    done; \
    for package in sdl2 libgvc libcgraph libcdt; do \
        if pkg-config --exists "$package"; then \
            echo "$package: OK"; \
        else \
            echo "$package: MISSING"; \
            status=1; \
        fi; \
    done; \
    exit "$status"
# Check Emscripten and WebAssembly build prerequisites.
doctor-web:
    @status=0; \
    for tool in emcmake em++ node python3; do \
        if command -v "$tool" >/dev/null; then \
            echo "$tool: OK"; \
        else \
            echo "$tool: MISSING"; \
            status=1; \
        fi; \
    done; \
    for path in \
        external/graphviz-wasm/include \
        external/graphviz-wasm/lib/libgvc.a \
        external/graphviz-wasm/lib/libcgraph.a \
        external/graphviz-wasm/lib/libcdt.a \
        external/graphviz-wasm/lib/libgvplugin_dot_layout.a \
        external/graphviz-wasm/lib/libvpsc.a \
        external/spot/wasm/include/spot/twa/twagraph.hh \
        external/spot/wasm/lib/libspot.a \
        external/spot/wasm/lib/libbddx.a; do \
        if [ -e "$path" ]; then \
            echo "$path: OK"; \
        else \
            echo "$path: MISSING"; \
            status=1; \
        fi; \
    done; \
    exit "$status"

# Report optional code-quality and coverage tools.
doctor-quality:
    @for tool in just clang-format clang-tidy gcov gcovr; do \
        if command -v "$tool" >/dev/null; then \
            echo "$tool: OK"; \
        else \
            echo "$tool: OPTIONAL, MISSING"; \
        fi; \
    done

# Check all required build tools and report optional quality tools.
doctor: doctor-native doctor-web doctor-quality

# Configure a native debug build.
configure-native:
    cmake -S . -B {{ native_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Build the native application and tests.
build-native: configure-native
    cmake --build {{ native_build_dir }}

# Build project-local native Spot into external/spot/native.
build-spot-native:
    @test -d external/spot/source || { echo "Missing external/spot/source. Run: git submodule update --init --recursive"; exit 1; }
    cmake -E make_directory external/spot/build-native external/spot/native
    cd external/spot/source && { test -x configure || autoreconf -fi; }
    cd external/spot/build-native && { test -f Makefile || ../source/configure --prefix="$PWD/../native" --disable-python --disable-shared --enable-static; }
    make -C external/spot/build-native/picosat -j"$(nproc)"
    make -C external/spot/build-native/buddy -j"$(nproc)"
    make -C external/spot/build-native/ltdl -j"$(nproc)"
    make -C external/spot/build-native/lib -j"$(nproc)"
    make -C external/spot/build-native/spot -j"$(nproc)"
    make -C external/spot/build-native/buddy install
    make -C external/spot/build-native/spot install

# Build project-local Emscripten Spot into external/spot/wasm.
build-spot-web:
    @test -d external/spot/source || { echo "Missing external/spot/source. Run: git submodule update --init --recursive"; exit 1; }
    @command -v emconfigure >/dev/null || { echo "emconfigure is missing. Activate emsdk first."; exit 1; }
    cmake -E make_directory external/spot/build-wasm external/spot/wasm
    cd external/spot/source && { test -x configure || autoreconf -fi; }
    cd external/spot/build-wasm && { test -f Makefile || gl_cv_func_sleep_works=yes emconfigure ../source/configure --host=wasm32-unknown-emscripten --prefix="$PWD/../wasm" --disable-python --disable-shared --enable-static; }
    emmake make -C external/spot/build-wasm/picosat -j"$(nproc)"
    emmake make -C external/spot/build-wasm/buddy -j"$(nproc)"
    emmake make -C external/spot/build-wasm/ltdl -j"$(nproc)"
    emmake make -C external/spot/build-wasm/lib -j"$(nproc)"
    emmake make -C external/spot/build-wasm/spot -j"$(nproc)"
    emmake make -C external/spot/build-wasm/buddy install
    emmake make -C external/spot/build-wasm/spot install

# Build and run all native tests.
test-native: build-native
    ctest --test-dir {{ native_build_dir }} --output-on-failure

# Run native tests matching a CTest regular expression.
test-one pattern: build-native
    ctest --test-dir {{ native_build_dir }} --output-on-failure -R "{{ pattern }}"

# Run the native application.
run-native: build-native
    ./{{ native_build_dir }}/{{ project }}

# Configure a native release build.
configure-native-release:
    cmake -S . -B {{ native_release_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

# Build the native release application.
build-native-release: configure-native-release
    cmake --build {{ native_release_build_dir }}

# Configure a WebAssembly debug build.
configure-web:
    emcmake cmake -S . -B {{ web_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON

# Build the WebAssembly debug application and operation worker.
build-web: configure-web
    cmake --build {{ web_build_dir }}

# Configure a WebAssembly build containing the test executables.
configure-web-tests:
    emcmake cmake -S . -B {{ web_test_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON

# Build and run all tests under Node through Emscripten.
test-web: configure-web-tests
    cmake --build {{ web_test_build_dir }}
    ctest --test-dir {{ web_test_build_dir }} --output-on-failure

# Build and run both native and WebAssembly test suites.
test-all: test-native test-web

# Configure the production-like WebAssembly build.
configure-web-release:
    emcmake cmake -S . -B {{ web_release_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON

# Build the production-like WebAssembly application and worker.
build-web-release: configure-web-release
    cmake --build {{ web_release_build_dir }}

# Serve the debug WebAssembly build.
serve: build-web
    @echo "Serving http://localhost:{{ web_port }}/index.html"
    python3 -m http.server {{ web_port }} -d {{ web_build_dir }}

# Serve the release WebAssembly build.
serve-release: build-web-release
    @echo "Serving http://localhost:{{ web_port }}/index.html"
    python3 -m http.server {{ web_port }} -d {{ web_release_build_dir }}

# Build and open the WebAssembly app with emrun.
emrun: build-web
    emrun {{ web_build_dir }}/index.html

# Format project-owned C++ files.
format:
    @command -v clang-format >/dev/null || { echo "clang-format is required."; exit 1; }

# Verify project-owned C++ formatting without changing files.
format-check:
    @command -v clang-format >/dev/null || { echo "clang-format is required."; exit 1; }

# Compile native debug targets with strict warnings promoted to errors, then test.
warnings-debug:
    cmake -S . -B {{ warnings_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON
    cmake --build {{ warnings_build_dir }}
    ctest --test-dir {{ warnings_build_dir }} --output-on-failure

# Compile native release targets with strict warnings promoted to errors, then test.
warnings-release:
    cmake -S . -B {{ warnings_release_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON
    cmake --build {{ warnings_release_build_dir }}
    ctest --test-dir {{ warnings_release_build_dir }} --output-on-failure

# Run strict warning gates in debug and optimized builds.
warnings: warnings-debug warnings-release

# Run clang-tidy while compiling project-owned native targets.
lint:
    cmake -S . -B {{ lint_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON -DREGEXTHESIS_ENABLE_CLANG_TIDY=ON
    cmake --build {{ lint_build_dir }}

# Build and test with AddressSanitizer and UndefinedBehaviorSanitizer.
sanitize:
    cmake -S . -B {{ sanitizer_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON -DREGEXTHESIS_ENABLE_SANITIZERS=ON
    cmake --build {{ sanitizer_build_dir }}
    ctest --test-dir {{ sanitizer_build_dir }} --output-on-failure

# Build, test, and generate terminal and HTML coverage reports with gcovr.
coverage:
    @command -v gcovr >/dev/null || { echo "gcovr is required for coverage reports."; exit 1; }
    cmake -S . -B {{ coverage_build_dir }} -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON -DREGEXTHESIS_ENABLE_COVERAGE=ON
    cmake --build {{ coverage_build_dir }}
    ctest --test-dir {{ coverage_build_dir }} --output-on-failure
    cmake -E make_directory coverage
    gcovr --root . --filter src --exclude external --print-summary --html-details coverage/index.html {{ coverage_build_dir }}
    @echo "Coverage report: coverage/index.html"

# List the generated debug web artifacts.
web-files: build-web
    find {{ web_build_dir }} -maxdepth 1 -type f | sort

# Package the deployable release web artifacts.
package-web: build-web-release
    cd {{ web_release_build_dir }} && cmake -E tar czf ../regex-thesis-web.tar.gz -- index.html index.js index.wasm operation-worker.js operation-worker.wasm style.css
    @echo "Web package: build/regex-thesis-web.tar.gz"

# Run the strict native quality gate.
check-native: format-check warnings

# Run WebAssembly tests and build release artifacts.
check-web: test-web build-web-release

# Run all repository quality gates.
check: doctor check-native check-web
    @echo "All checks completed."

# Remove all recipe-managed build and report artifacts.
clean:
    cmake -E remove_directory build
    cmake -E remove_directory coverage

# Remove native debug artifacts.
clean-native:
    cmake -E remove_directory {{ native_build_dir }}

# Remove WebAssembly debug artifacts.
clean-web:
    cmake -E remove_directory {{ web_build_dir }}

# Rebuild native and WebAssembly debug applications from clean recipe-managed trees.
rebuild: clean build-native build-web
