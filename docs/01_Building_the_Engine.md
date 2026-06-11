# 1. Building the Engine

*This tutorial walks you through compiling the Sandbox Engine from source, ensuring all dependencies are met.*

**Conclusion first:** To build the engine, you need a C++23 compiler, CMake 3.20+, and the `flatc` compiler available on your system. You will generate the necessary FlatBuffer headers, run CMake configuration, and compile the engine and test suites.

## Prerequisites
*   **C++23 Compiler:** GCC 12+, Clang 15+, or MSVC 19.38+.
*   **CMake:** Version 3.20 or newer.
*   **FlatBuffers (`flatc`):** The FlatBuffer schema compiler.

## Step 1: Generate FlatBuffer Headers
Before compiling C++ code, the engine needs the C++ headers generated from the `.fbs` schema files located in `src/schemas/`. 
If your build script does not do this automatically, you must generate them manually:

```bash
cd sandbox
flatc --cpp -o include/sandbox/generated/schemas src/schemas/*.fbs
```

## Step 2: CMake Configuration
Create a build directory and configure the project. The engine pulls down dependencies like `flecs`, `spdlog`, and `physfs` automatically via CMake `FetchContent` or submodules.

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Step 3: Build
Compile the engine, the launcher, and the test suites.

```bash
cmake --build . -j$(nproc)
```

## Step 4: Verify the Build
Run the unit tests to ensure your C-ABI boundaries and ECS wrappers are functioning correctly on your host architecture.

```bash
./bin/sandbox_unit_tests
```
If you see `All tests passed`, you are ready to build plugins!
