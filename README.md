# RegexThesis

RegexThesis provides **Regex Workbench**, an interactive regular-expression application written
in C++20. It supports both finite-word and omega regular expressions, constructs and lays out
finite or Büchi automata, rewrites expressions, and compares their languages. The same application
runs as a native Linux program and as a fully client-side WebAssembly application.

The web version sends no expressions or computation results to a backend. Parsing, automata
algorithms, language comparison, rewriting, and Graphviz layout all execute locally in the
browser. A web server is needed only to serve the static application files.

## What the application can do

- Translate a finite expression into an epsilon-free NFA or a minimal DFA, or an omega expression
  into a reduced state-based NBA or a minimal state-based DBA when one exists.
- Render automata as interactive Graphviz-derived diagrams with draggable states.
- Rewrite and simplify finite and omega expressions while optionally removing complement,
  intersection, power, plus, or Sigma abbreviations. Rewritten expressions are language-equivalent
  but not guaranteed to be size-minimal.
- Compare two languages as equivalent, complementary, disjoint, subset-related, or overlapping.
- Show shortest finite witnesses or ultimately periodic omega witnesses for the left-only,
  right-only, intersection, and neither regions.
- Generate random finite or omega expressions with configurable weights and maximum depth.
- Cancel expensive work manually or automatically after a configurable deadline.
- Highlight invalid inputs and report syntax errors with actionable source positions

The design keeps the mathematical layers independent of Dear ImGui and isolates expensive work
from the UI. Core algorithms are shared by the native and WebAssembly builds and are exercised by
the same test suite on both platforms.

## Interface

The main workspace follows a focused, task-oriented layout:

1. Select **Finite words** or **Infinite words**.
2. Choose the **Translate**, **Rewrite**, or **Compare** tab.
3. Enter one or two expressions and enable **Σ Additional alphabet** only when extra symbols are
   needed.
4. Press the visible operation button or press Enter in an expression field.

Expression symbols are included in the active alphabet automatically. Rewrite always presents its
operator-expansion choices. Generating a random expression immediately runs the chosen operation.
Result cards show operation output or errors.
Automata offer Graph/Text navigation, with copying available in Text view, and moved states
immediately update their incident edge geometry without recomputing the Graphviz layout.
Comparison results keep all four regions visible,
distinguish empty regions from missing output, and use consistent blue, green, violet, and amber
region/witness colors. The header provides settings and an expanded guide. The
light/dark theme switch remains in Settings.

## Finite regular-expression syntax

Terminals are ASCII letters and digits. Whitespace is ignored. `&&` and `||` are accepted as
aliases for `&` and `|`.

| Syntax        | Meaning |
|---------------| --- |
| `a`, `B`, `7` | One terminal symbol |
| `ε`           | The empty word |
| `∅`           | The empty language |
| `Σ`           | Any single symbol from the active alphabet |
| `(r)`         | Grouping |
| `r^n`         | Exactly `n` concatenated copies; `n` is an unsigned integer |
| `r*`          | Kleene star |
| `r+`          | One or more copies |
| `!r`, `~r`    | Language complement |
| `!!r`, `!!!r` | Repeated complement prefixes, simplified by parity |
| `rs`          | Concatenation |
| `r&s`         | Intersection |
| `r\|s`        | Alternation |

Precedence from strongest to weakest is atom, power, repetition, complement, concatenation,
intersection, then alternation. Complement prefixes may be repeated: `!!a` is simplified to `a`,
and `!!!a` is simplified to `!a`.

The active alphabet is the union of terminals found in the input expressions and the
symbols entered in the alphabet field.

The input editor offers `\sigma`, `\epsilon`, and `\emptyset` completions for the corresponding
Unicode symbols.

## Omega regular-expression syntax

Omega mode describes languages of infinite words. In the table below, `r` and `s` are finite
regular expressions, while `R` and `S` are omega regular expressions.

| Syntax        | Meaning |
|---------------| --- |
| `r^ω`         | Infinite concatenation of finite words selected from `r` |
| `rR`          | A finite word from `r`, followed by an infinite word from `R` |
| `∅`           | The empty omega language |
| `Σ^ω`         | The universal omega language over the active alphabet |
| `(R)`         | Grouping |
| `!R`, `~R`    | Complement over all infinite words on the active alphabet |
| `!!R`, `!!!R` | Repeated complement prefixes, simplified by parity |
| `R&S`         | Intersection |
| `R\|S`        | Alternation |

Omega expressions are type-checked by the grammar: every alternation or intersection branch must
itself describe infinite words. Thus `a|b^ω` is invalid, write `a^ω|b^ω`. Concatenation binds more
strongly than intersection and alternation. For example, `ab^ω` means a finite `a` prefix followed
by infinitely many `b` symbols, whereas `(ab)^ω` repeats the block `ab` forever. Composite finite
bases of omega powers should be parenthesized, as in `(ab)^ω` or `(a|b)^ω`.

The active alphabet has the same definition as in finite mode and is semantically important for
omega complement. Internally, Spot receives a Boolean encoding of the active characters, and
complement operations are interpreted relative to the application's active alphabet rather than to
unrestricted internal Boolean valuations. The input editor additionally offers `\omega` in omega
mode.

### Random expression generation

The dice button generates an expression for the active word domain. In finite mode, it builds a
regular-expression AST using weighted choices for growth, leaves, and operators. In Infinite words
mode, it builds an omega-expression AST directly. The omega generator can create `S^ω`,
finite-prefix/period expressions such as `RS^ω`, omega alternation, omega intersection, omega
complement, `∅`, and `Σ^ω`.

The omega generator reuses finite generator configurations for finite subexpressions. Prefixes and
periods are configured separately so that prefixes may stay permissive while periods can be biased
toward visible terminal progress. Degenerate omega powers such as `ε^ω` and `∅^ω` are still valid
generated edge cases if enabled by the configuration.

### Operation semantics
Translate offers **NFA / Minimal DFA** for finite words and **NBA / Minimal DBA** for infinite
words. Both Büchi modes display accepting states as double circles. NBA mode reduces the automaton
before and after conversion to state acceptance, without guaranteeing a global minimum.

DBA mode decides whether the language admits a deterministic Büchi automaton. Successful DBA results have the
minimum number of states, allowing missing transitions to reject and omitting rejecting sinks.

Rewrite applies language-preserving algebraic normalization around the requested unabbreviations.
Direct expansions such as plus, power, and Sigma remain structural. Automata conversion is used
only for complement or intersection nodes when removing those operators actually requires a
semantic transformation. Omega rewrites additionally verify after normalization that requested
abbreviations remain removed. Rewrites are limited to 4,096 output AST nodes.

Compare uses a common alphabet for both inputs, classifies their language relation, and detects
empty and universal languages. Finite mode computes shortest region witnesses and displays the
empty word as `ε`, omega mode uses Spot and displays witnesses as `prefix(cycle)^ω`.

## Quick start

[`just`](https://github.com/casey/just) is the recommended command runner. On a fresh checkout,
initialize the Spot submodule and build the project-local native Spot installation first:

```sh
git submodule update --init --recursive
just build-spot-native
just doctor-native
just test-native
just run-native
```

To build and serve the browser application:

```sh
just build-spot-web
just serve-release
```

Then open <http://localhost:8080/index.html>. `just serve` uses the slower debug WebAssembly build
with additional Emscripten runtime checks. Activate the Emscripten SDK before running either web
command.

Run `just` or `just --list` to see every available recipe.

## Dependencies

The project requires a C++20 compiler and CMake 3.20 or newer. Ninja is used by the documented
commands.

### Native build

- SDL2
- OpenGL ES 2 development files
- Graphviz development libraries: `libgvc`, `libcgraph`, and `libcdt`
- `pkg-config`
- Autoconf, Automake, Libtool, and Make for the one-time local Spot build
- POSIX process APIs and `/proc/self/exe` for locating the native operation worker

The current native runner therefore targets Linux. Install the development packages supplied by
your distribution, then use `just doctor-native` to check the discoverable prerequisites.

### WebAssembly build

- Emscripten (`emcmake`, `em++`)
- Node.js for the WebAssembly test executables
- Python 3 for the local static-file server recipe
- Autoconf, Automake, Libtool, and Make for the one-time Emscripten Spot build

Activate the Emscripten SDK environment before configuring. `just doctor-web` checks the toolchain
and the required prebuilt Graphviz archives.

### Online access
 Alternatively, you can access the application online at https://catundercover.github.io/RegexWorkbench/

### Repository and fetched dependencies

- Dear ImGui, SDL/OpenGL backends, and fonts are stored in `external/imgui`.
- MATA is stored in `external/mata`.
- [Spot](https://spot.lre.epita.fr/) is pinned as the `external/spot/source` submodule. The
  `build-spot-native` and `build-spot-web` recipes install it into ignored project-local prefixes.
- The Emscripten Graphviz headers and static libraries are stored in `external/graphviz-wasm`.
- [lexy](https://lexy.foonathan.net/) is pinned by commit and fetched by CMake on the first
  configure unless a local FetchContent source override is supplied.

`just doctor-quality` separately reports optional development tools: clang-format, clang-tidy,
gcov, and gcovr.

## Building without `just`

### Native debug build and tests

```sh
just build-spot-native
cmake -S . -B build/native-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON
cmake --build build/native-debug
ctest --test-dir build/native-debug --output-on-failure
./build/native-debug/RegexThesis
```

`RegexOperationWorker` must stay beside `RegexThesis`; the build already places it there.

### WebAssembly release build

```sh
just build-spot-web
emcmake cmake -S . -B build/web-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DREGEXTHESIS_WARNINGS_AS_ERRORS=ON
cmake --build build/web-release
python3 -m http.server 8080 -d build/web-release
```

The deployable files are:

```text
index.html
index.js
index.wasm
operation-worker.js
operation-worker.wasm
style.css
```

Keep them in the same directory. The host should serve `.wasm` files as `application/wasm`.
SharedArrayBuffer and cross-origin-isolation headers are not required because the application uses
an isolated Web Worker instead of pthreads. `just package-web` creates
`build/regex-thesis-web.tar.gz` containing the production files.

To publish the same artifacts to GitHub Pages through a dedicated `web-release` branch, see
[`docs/web-release.md`](docs/web-release.md).

To use a different Emscripten Graphviz prefix:

```sh
emcmake cmake -S . -B build/web-release -G Ninja \
  -DREGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT=/path/to/graphviz-prefix
```

To use an existing compatible Spot installation instead of a recipe-managed prefix, pass
`-DREGEXTHESIS_SPOT_ROOT=/path/to/spot-prefix` when configuring. The prefix must contain Spot
headers plus `libspot` and `libbddx` built for the selected native or WebAssembly toolchain.

## Execution and cancellation model

The complete expensive path—from parsing through automata work to Graphviz layout—runs outside the
UI thread:

```text
MainWindow
  -> OperationCoordinator
  -> OperationController (deadline and cancellation)
  -> platform OperationRunner
  -> operation worker
  -> validation -> regex/automata algorithms -> graph layout
```

In the browser, the operation runner explicitly creates `operation-worker.js` through Emscripten's
worker API, giving the worker an independent WASM instance. A successful worker is reused to avoid
repeated startup cost; cancellation, timeouts, and protocol failures terminate it. No POSIX API or
backend request is involved.

The native build starts a local `RegexOperationWorker` process and exchanges a size-prefixed binary
request/result payload through pipes. Terminating the process provides reliable cancellation even
when a third-party algorithm cannot cooperatively poll a stop token.

Only one operation is active at a time. The default deadline is five seconds and the Settings
window allows values from 1 to 60 seconds. A Cancel button is shown while work is running.

## Settings and safety limits

The Settings window exposes values that are safe and useful to change interactively:

- operation timeout
- graph canvas height
- light or dark application theme
- finite random-generator growth, leaf, operator weights, and maximum depth
- omega random-generator growth, leaf, operator weights, maximum depth, and separate finite
  prefix/period generator settings.

Settings currently last for the application session. Invalid combinations are normalized, for
example, generator weights can never leave the generator without a usable choice.

Rendered automata are interactive: dragged states remain exactly where they are placed, use the mouse wheel or the
`-`/`+` controls to zoom, choose **Fit view** to restore the fitted viewport without discarding
moved states, or choose **Reset layout** to restore the original Graphviz result. The graph canvas
uses the centralized Dear ImGui theme for its background, lines, and labels, while automaton state
fills use fixed accent colors for normal, hovered, and dragged states. `web/style.css` styles only
the browser host and loading surface.

Hard correctness, transport, and rendering ceilings remain source-level invariants rather than UI
settings:

| Limit | Default | Location |
| --- | ---: | --- |
| Regex input | 4,096 bytes | `src/app/operations/OperationLimits.hpp` |
| Rendered automaton | 250 states / 900 transitions | `src/app/operations/OperationLimits.hpp` |
| Generated DOT | 2 MiB | `src/app/operations/OperationLimits.hpp` |
| Rewritten expression | 4,096 AST nodes | `src/automata/rewrite/RegexRewriter.hpp` |
| Worker payload | 16 MiB | `src/worker/protocol/ProtocolLimits.hpp` |

## Project structure

```text
src/
├── regex/       AST, parsing, formatting, inspection, simplification, generation
├── automata/    NFA/Büchi models, MATA/Spot conversions, translation, rewriting, comparison, DOT
├── graph/       renderer-neutral geometry plus Graphviz layout extraction
├── app/         operation use cases, settings, platform integration, SDL/ImGui runtime
├── worker/      platform worker entry points and binary request/result protocol
└── ui/          MainWindow composition and focused input, operation, result, graph, settings views
web/             Emscripten shell and browser clipboard/input bridge
tests/           native/WASM unit, property, protocol, and integration tests
benchmarks/      representative end-to-end operation benchmark
cmake/           dependency checks and reusable project-quality options
external/        vendored dependencies, the Spot submodule, and local Spot build prefixes
```

## Tests and code quality

The project currently defines 18 CTest C++ executables covering:

- regex parsing, diagnostics, formatting, normalization, simplification, inspection, and random
  generation
- finite and Büchi automata model behavior, conversions, language preservation, rewrite behavior,
  comparison, Spot alphabet semantics, and bounded generated-expression properties
- Graphviz layout extraction, draggable-node state, spline deformation, and self-loop geometry
- operation validation, execution, timeout, cancellation, and coordination
- settings normalization
- binary protocol round trips, truncation, malformed tags, trailing bytes, overflow, and payload
  limits

The WebAssembly test configuration additionally registers a Node.js-based browser bridge test for
clipboard and paste routing logic used by the browser build.

Run the same core algorithm and application-logic tests natively and under Node/WebAssembly:

```sh
just test-native
just test-web
just test-all
```

Useful quality gates are:

```sh
just format-check   # Verify clang-format without modifying files
just warnings       # Debug and Release builds with warnings as errors
just lint           # clang-tidy with diagnostics promoted to errors
just sanitize       # AddressSanitizer + UndefinedBehaviorSanitizer
just coverage       # Tests plus terminal/HTML gcovr reports
just check          # Prerequisites, native gates, WASM tests, release WASM build
```

The sanitizer configuration keeps leak detection active. It narrowly suppresses process-global
fontconfig/Pango caches retained by the native Graphviz plugins.

`just coverage` requires gcovr and writes `coverage/index.html`. Rendering and SDL lifecycle code
is primarily verified by build and interactive integration checks; the deterministic regex,
automata, graph, operation, protocol, settings, and UI-logic layers are the focus of line coverage.


## CMake options

| Option | Default | Purpose |
| --- | --- | --- |
| `REGEXTHESIS_WARNINGS_AS_ERRORS` | `OFF` | Promote project-owned compiler warnings to errors |
| `REGEXTHESIS_ENABLE_CLANG_TIDY` | `OFF` | Run clang-tidy during native compilation |
| `REGEXTHESIS_ENABLE_SANITIZERS` | `OFF` | Enable native ASan and UBSan |
| `REGEXTHESIS_ENABLE_COVERAGE` | `OFF` | Enable native gcov instrumentation |
| `REGEXTHESIS_EMSCRIPTEN_GRAPHVIZ_ROOT` | bundled path | Override the WASM Graphviz prefix |
| `REGEXTHESIS_SPOT_ROOT` | project-local native/WASM prefix | Override the Spot installation |

Sanitizer and coverage instrumentation intentionally require separate build trees. Third-party
ImGui, MATA, lexy, and Graphviz code is kept outside the project lint policy so warnings remain
actionable.

## Troubleshooting

- If Spot is missing, initialize submodules and run `just build-spot-native` or, with emsdk active,
  `just build-spot-web`. Remove the matching `external/spot/build-*` directory before retrying a
  build with a materially different toolchain configuration.
- If CMake cannot find lexy, check network access on the first configure or pass
  `-DFETCHCONTENT_SOURCE_DIR_LEXY=/path/to/lexy`.
- If native Graphviz is missing, verify `pkg-config --exists libgvc libcgraph libcdt`.
- If `RegexOperationWorker` cannot start, keep it next to the native `RegexThesis` executable and
  run on Linux with `/proc` mounted.
- If the web page is blank when opened directly, serve the build directory over HTTP instead of
  using a `file://` URL.
- An Emscripten warning about an unexpected Binaryen version indicates a mismatched SDK/tool pair;
  activate one consistent emsdk installation before rebuilding.
- If `just coverage` reports that gcovr is missing, install gcovr for the active development
  environment; builds and tests do not otherwise depend on it.
