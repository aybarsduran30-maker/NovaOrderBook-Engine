# ---------------------------------------------------------------------------
# NovaOrderBook Engine - reproducible Linux build
#
# Two stages:
#   1. "build"   - full toolchain, compiles every CMake target and runs ctest.
#   2. "runtime" - slim image with just the binaries and the Python module.
#
# The split earns its keep: the build stage needs build-essential, cmake, git
# and the FetchContent checkouts of googletest/pybind11, which is all pure
# build scaffolding. Measured on arm64: build stage 621 MB, runtime 148 MB.
#
# Build and run (see the Docker section of BUILD.md for more):
#   docker build -t nova-orderbook .
#   docker run --rm nova-orderbook                  # demo CLI
#   docker run --rm nova-orderbook nova_benchmark   # 1M order benchmark
#
# Base image is pinned to a dated tag so rebuilds are reproducible.
# debian:bookworm gives us gcc 12.2 (full C++20) and CMake 3.25.1, comfortably
# above the CMakeLists.txt floor of 3.16.
# ---------------------------------------------------------------------------


# =============================== STAGE 1: build ============================
FROM debian:bookworm-20260824-slim AS build

# build-essential -> g++ 12.2, make
# cmake           -> 3.25.1
# git + ca-certificates -> CMakeLists.txt uses FetchContent to clone
#                          googletest v1.14.0 and pybind11 v2.12.0 from
#                          github.com at *configure* time, so this stage
#                          needs outbound HTTPS. On an air-gapped builder,
#                          vendor the sources and pass
#                          -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=... and
#                          -DFETCHCONTENT_SOURCE_DIR_PYBIND11=... instead.
# python3-dev     -> headers required by pybind11_add_module(nova_engine ...).
#                    Plain python3 is not enough; the -dev package is what
#                    provides Python.h.
RUN apt-get update && apt-get install --no-install-recommends -y \
        build-essential \
        cmake \
        git \
        ca-certificates \
        python3 \
        python3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy only what the C++ build actually reads. Keeping README/.git and the
# Python scripts out of the context means editing them does not bust this
# layer, and it keeps the build honest about its inputs.
COPY CMakeLists.txt ./
COPY include/ include/
COPY src/ src/
COPY tests/ tests/

# Set the build type explicitly. CMakeLists.txt does not set one, and Make and
# Ninja default to an empty build type, which means no -O flag at all. Visual
# Studio, a multi-config generator, picks one at build time instead.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Builds all five targets: nova_core, nova_tests, nova_cli, nova_engine,
# nova_benchmark.
RUN cmake --build build --parallel "$(nproc)"

# Build-time verification: the image cannot be produced if the suite is red.
#
# Two things to know about running this suite in a container:
#  - Each OrderBook constructor heap-allocates ~61 MB and every one of the
#    7 gtest fixtures builds a fresh one, so give the builder >= 512 MB.
#  - CMakeLists.txt uses gtest_discover_tests(), which executes nova_tests on
#    the build machine during `cmake --build` to enumerate cases. That is fine
#    for a native build like this one. If you ever cross-build (for example
#    `--platform linux/amd64` on an arm host, which runs under emulation),
#    add DISCOVERY_MODE PRE_TEST to that call in CMakeLists.txt.
RUN ctest --test-dir build --output-on-failure


# ============================== STAGE 2: runtime ===========================
FROM debian:bookworm-20260824-slim AS runtime

# python3 must be the same 3.11 as the build stage: the extension module is
# named nova_engine.cpython-311-<arch>-linux-gnu.so and is ABI-locked to it.
# Using the identical pinned base image on both stages is what guarantees that.
# libstdc++6 is what the C++ binaries link against at runtime.
RUN apt-get update && apt-get install --no-install-recommends -y \
        python3 \
        libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/nova

# CMakeLists.txt declares no install() rules, so artifacts are copied straight
# out of the build tree. The .so filename embeds the CPU arch, hence the glob.
COPY --from=build /src/build/nova_cli       /usr/local/bin/nova_cli
COPY --from=build /src/build/nova_benchmark /usr/local/bin/nova_benchmark
COPY --from=build /src/build/nova_engine*.so /opt/nova/

# So `import nova_engine` works from any working directory.
ENV PYTHONPATH=/opt/nova

# Smoke-test what actually ships, not what was built.
# nova_cli reads no stdin and exits 0, so it is safe to run unattended.
# The Python check crosses a buy against a sell and asserts a trade happened.
#
# Note the price: the book is a fixed array covering MIN_PRICE..MAX_PRICE
# (10000..20000, include/OrderBook.hpp:9-10) and addOrder silently returns for
# anything outside that band, so a smoke test using a small price like 100
# would pass through without ever matching and report zero trades.
RUN nova_cli > /dev/null \
    && python3 -c "\
import nova_engine; \
b = nova_engine.OrderBook(); \
b.addOrder(1, nova_engine.Side.BUY, 15000, 10); \
b.addOrder(2, nova_engine.Side.SELL, 15000, 10); \
assert b.getTradeCount() >= 1, 'expected a trade'; \
print('nova_engine import OK, trades:', b.getTradeCount())"

# Run as a non-root user; nothing here needs privileges.
RUN useradd --create-home --shell /usr/sbin/nologin nova
USER nova

# Default: the demo CLI. The benchmark is also available as a second
# entrypoint (see the run examples at the top of this file).
#
# Not wired up: engine_pipeline.py, metrics_consumer.py, simulate_stream.py
# and test_engine.py. They need a Windows .dll, a Redis server, or an
# interactive terminal, so they are outside what a headless Linux image can
# run. BUILD.md has the details.
CMD ["nova_cli"]
