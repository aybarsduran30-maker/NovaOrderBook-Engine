# Building NovaOrderBook

This document covers building the project from a fresh clone on Linux, macOS,
and Windows.

The project was developed on Windows and builds on all three platforms. CMake
is the primary build system and drives every target. `setup.py` is a second,
Python-only path that builds just the extension module.

## What gets built

`CMakeLists.txt` defines five targets.

| Target | Kind | Source | Output |
| --- | --- | --- | --- |
| `nova_core` | Static library | `src/OrderBook.cpp` | `libnova_core.a` / `nova_core.lib` |
| `nova_tests` | GoogleTest executable | `tests/test_orderbook.cpp` | `nova_tests` |
| `nova_cli` | Executable | `src/main.cpp` | `nova_cli` |
| `nova_engine` | Python extension module | `src/bindings.cpp` | `nova_engine.<abi>.so` / `.pyd` |
| `nova_benchmark` | Executable | `tests/benchmark.cpp` | `nova_benchmark` |

Everything else links against `nova_core`.

## Prerequisites

| Requirement | Minimum | Notes |
| --- | --- | --- |
| CMake | 3.16 | Declared in `CMakeLists.txt`. Also builds on 4.x. |
| C++ compiler | GCC 10, Clang 11, or MSVC from Visual Studio 2022 | `CMAKE_CXX_STANDARD 20` is set with `REQUIRED ON`, so the compiler must accept C++20. |
| Build tool | Make, Ninja, or MSBuild | Any generator CMake supports. |
| Git | any recent version | Needed at configure time, see below. |
| Python | 3.8 or newer, plus development headers | Only needed for the `nova_engine` target. |
| Network access | outbound HTTPS to `github.com` | Needed at configure time, see below. |
| Memory | around 512 MB for the build machine | Each `OrderBook` allocates roughly 61 MB, and the test suite constructs one per test case. |

### Network access at configure time

`CMakeLists.txt` uses `FetchContent` to clone two dependencies from GitHub when
you configure:

- googletest `v1.14.0`
- pybind11 `v2.12.0`

There is no vendored copy and no `find_package` fallback, so the first
configure needs network access. If you build somewhere without it, download
both repositories yourself and point CMake at them:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/googletest \
  -DFETCHCONTENT_SOURCE_DIR_PYBIND11=/path/to/pybind11
```

### Python headers

The `nova_engine` target calls `pybind11_add_module`, which needs `Python.h`.
On Debian and Ubuntu, install `python3-dev`; the `python3` package alone does
not include the headers. On Fedora and RHEL, install `python3-devel`. If you
only want the C++ targets, you can skip Python entirely and build the specific
targets you need — see [Building a subset](#building-a-subset).

### Installing the prerequisites

Debian or Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git ca-certificates python3 python3-dev
```

Fedora or RHEL:

```bash
sudo dnf install -y gcc-c++ make cmake git python3 python3-devel
```

macOS, with Homebrew and the Xcode command line tools:

```bash
xcode-select --install
brew install cmake
```

Windows: install Visual Studio 2022 with the "Desktop development with C++"
workload, plus [CMake](https://cmake.org/download/),
[Git](https://git-scm.com/download/win), and
[Python](https://www.python.org/downloads/windows/). The Visual Studio
installer can supply CMake and Git if you prefer.

## Build on Linux and macOS

```bash
git clone https://github.com/papagala/NovaOrderBook-Engine.git
cd NovaOrderBook-Engine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

Adjust `--parallel` to the number of cores you want to use.

`-DCMAKE_BUILD_TYPE=Release` is worth passing explicitly. `CMakeLists.txt` does
not set a build type, and Make and Ninja default to an empty one, which means
no optimization flags are applied at all. Visual Studio picks a configuration
at build time instead, which is why this only affects single-configuration
generators.

All five targets land directly in `build/`:

```
build/libnova_core.a
build/nova_tests
build/nova_cli
build/nova_benchmark
build/nova_engine.cpython-311-x86_64-linux-gnu.so
```

The extension module filename embeds your Python version and platform, so it
will not match the example above exactly.

## Build on Windows

From "Developer PowerShell for VS 2022", or any shell where `cmake` is on
`PATH`:

```powershell
git clone https://github.com/papagala/NovaOrderBook-Engine.git
cd NovaOrderBook-Engine
cmake -S . -B build
cmake --build build --config Release --parallel 4
```

Visual Studio is a multi-configuration generator. You choose the configuration
at build time with `--config`, not at configure time with `CMAKE_BUILD_TYPE`.

Artifacts land in a configuration subdirectory:

```
build\Release\nova_cli.exe
build\Release\nova_benchmark.exe
build\Release\nova_tests.exe
build\Release\nova_engine.cp311-win_amd64.pyd
```

MinGW-w64 also works. Pass the generator explicitly and it behaves like the
Linux build, with output directly in `build\`:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

## Run the tests

`tests/test_orderbook.cpp` holds seven GoogleTest cases. `CMakeLists.txt`
registers them with CTest via `gtest_discover_tests`.

Linux and macOS:

```bash
ctest --test-dir build --output-on-failure
```

Windows:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

You can also run the binary directly, which gives you the GoogleTest flags such
as `--gtest_filter`:

```bash
./build/nova_tests
```

One thing to know about the CMake setup: `gtest_discover_tests` runs
`nova_tests` on the build machine during `cmake --build` in order to enumerate
the cases. That is fine for a native build. If you cross-compile, or build for
another architecture under emulation, add `DISCOVERY_MODE PRE_TEST` to that
call in `CMakeLists.txt` so discovery is deferred to test time.

## Run the CLI

`nova_cli` submits a fixed set of orders, prints the resulting book state, and
exits. It reads no input.

```bash
./build/nova_cli
```

```powershell
.\build\Release\nova_cli.exe
```

## Run the benchmark

`nova_benchmark` submits 1,000,000 generated orders from a fixed seed and
prints a timing summary. It takes no arguments and reads no input.

```bash
./build/nova_benchmark
```

```powershell
.\build\Release\nova_benchmark.exe
```

The target is built by `cmake --build` but is not registered with CTest, so
`ctest` will not run it. Invoke it directly.

Build it with `-DCMAKE_BUILD_TYPE=Release` (or `--config Release` on Windows).

## The Python module

The module is named `nova_engine`. That name is fixed by
`PYBIND11_MODULE(nova_engine, m)` in `src/bindings.cpp`, so the built file must
begin with `nova_engine` and must be on `sys.path` for `import nova_engine` to
work.

There are two ways to build it. Pick one.

### Option A: CMake

The CMake build already produced the module. Put its directory on `PYTHONPATH`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
PYTHONPATH=$PWD/build python3 -c "import nova_engine; print(nova_engine.__file__)"
```

```powershell
$env:PYTHONPATH = "$PWD\build\Release"
python -c "import nova_engine; print(nova_engine.__file__)"
```

Make sure the Python you import with is the same one CMake found at configure
time. The configure output prints which interpreter and library it picked. An
extension built against one minor version will not import into another.

### Option B: pip

`setup.py` builds the same module without CMake. It imports `pybind11` at the
top of the file, so pybind11 must already be installed before the build starts.
The repository has no `pyproject.toml`, which means an isolated PEP 517 build
would not provide it. Install pybind11 first and disable isolation:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install "pybind11==2.12.0"
pip install --no-build-isolation .
```

The pin matches the `GIT_TAG v2.12.0` in `CMakeLists.txt`, which keeps both
build paths on the same pybind11 version.

To build the module in the source tree instead of installing it:

```bash
python3 setup.py build_ext --inplace
```

That drops `nova_engine.<abi>.so` in the repository root, where `import
nova_engine` finds it as long as you run Python from that directory.

Two notes on this path:

- `setup.py` passes `-std=c++20` and `-O3`. Those are GCC and Clang spellings.
  MSVC uses `/std:c++20` and `/O2` and will not act on them, so use the CMake
  path on Windows unless you are building with MinGW-w64.
- `setup.py` also writes intermediates into `build/`. The subdirectory names do
  not collide with CMake's, so the two can share the directory. If you would
  rather keep them apart, configure CMake into a different directory, for
  example `cmake -S . -B cmake-build`.

### Check that it works

```bash
python3 -c "
import nova_engine
book = nova_engine.OrderBook()
book.addOrder(1, nova_engine.Side.BUY, 15000, 10)
book.addOrder(2, nova_engine.Side.SELL, 15000, 10)
print('best bid:', book.getBestBid())
print('trades:', book.getTradeCount())
"
```

This prints a trade count of 1.

Use a price inside the book's range. `OrderBook` covers ticks 10000 through
20000 inclusive (`MIN_PRICE` and `MAX_PRICE` in `include/OrderBook.hpp`), and
`addOrder` returns without doing anything for a price outside that range or a
count of zero. A smoke test written with a price such as 100 will report zero
trades and look like a build failure when the build is fine.

The bound surface is in `src/bindings.cpp`: `Side` with `BUY` and `SELL`,
`Trade`, and `OrderBook` with `addOrder`, `cancelOrder`, `getBestBid`,
`getBestAsk`, and `getTradeCount`.

## Docker

A two-stage `Dockerfile` is in the repository root. The build stage compiles
every target and runs the test suite; the runtime stage carries only `nova_cli`,
`nova_benchmark`, and the extension module.

```bash
docker build -t nova-orderbook .
docker run --rm nova-orderbook
```

The default command runs `nova_cli`. To run the benchmark instead:

```bash
docker run --rm nova-orderbook nova_benchmark
```

To use the Python module:

```bash
docker run --rm nova-orderbook python3 -c "import nova_engine; print(nova_engine.OrderBook())"
```

The build stage clones googletest and pybind11 from GitHub, so `docker build`
needs network access. Give the builder at least 512 MB of memory. If you build
for a different architecture than the host, see the note about
`gtest_discover_tests` in [Run the tests](#run-the-tests).

## Building a subset

To skip the Python module and its dependency on Python headers, build the
targets you want by name:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4 --target nova_cli nova_tests nova_benchmark
```

Configuring still requires Python, because `pybind11_add_module` is evaluated
at configure time. Only the compile step is skipped.

## Scripts in the repository root

The `.py` files at the top level are not part of the build and are not run by
CI. They have their own requirements:

- `simulate_stream.py` imports `nova_engine` and renders a terminal display. It
  adds `build/Release` to `sys.path`, which is the Visual Studio layout, so it
  finds the module after a Windows CMake build. On Linux and macOS, set
  `PYTHONPATH` to your build directory or install the module first. It clears
  the screen and writes ANSI escapes, so run it in a real terminal.
- `engine_pipeline.py` and `metrics_consumer.py` need the `redis` package and a
  Redis server on `localhost:6379`. `metrics_consumer.py` loops until you
  interrupt it. `engine_pipeline.py` also loads `nova_engine.dll` through
  `ctypes`; that is a separate C ABI from the pybind11 module, and no CMake
  target builds it.
- `test_engine.py` is a standalone script rather than a test module. It loads
  `nova_engine.dll` through `ctypes` and is Windows-only.

None of these are needed to build or test the engine.

## Troubleshooting

**`Could NOT find Python` or `Python.h: No such file or directory`** — install
the Python development headers (`python3-dev` or `python3-devel`), or build
only the C++ targets as shown above.

**`fatal: unable to access 'https://github.com/...'` during configure** —
`FetchContent` cannot reach GitHub. Use the
`FETCHCONTENT_SOURCE_DIR_*` variables shown in
[Network access at configure time](#network-access-at-configure-time).

**`ModuleNotFoundError: No module named 'pybind11'` from `pip install .`** —
pip is building in an isolated environment that does not have pybind11.
Install it first and pass `--no-build-isolation`, as in Option B.

**`ModuleNotFoundError: No module named 'nova_engine'`** — the module directory
is not on `sys.path`. On a single-configuration build the file is in `build/`;
on Windows it is in `build\Release\`. Set `PYTHONPATH` accordingly, or install
the module with pip.

**`dynamic module does not define module export function
(PyInit_nova_orderbook)`** — the built file was named something other than
`nova_engine`. pybind11 derives the init symbol from the module name in
`src/bindings.cpp`, so the filename has to match. Rebuild with CMake or
`setup.py`, both of which name it correctly.

**`ImportError: ... undefined symbol` or a version mismatch on import** — the
module was built against a different Python than the one importing it. Rebuild
with the interpreter you intend to use.

**A stale CMake cache after changing compilers or Python versions** — delete
the build directory and configure again. The cache stores absolute paths to the
toolchain it found the first time.

## Verified configurations

The CMake build, the test suite, and the Python import were run end to end on:

- macOS 15 on arm64, Apple Clang 21, CMake 4.2.3, Python 3.13
- Debian bookworm on arm64, GCC 12.2, CMake 3.25.1, Python 3.11, via the
  `Dockerfile` in this repository

`.github/workflows/ci.yml` runs the configure, build, test, and CLI smoke steps
on `ubuntu-latest` with Python 3.11 for every push and pull request to `main`.
