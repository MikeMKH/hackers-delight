# hackers-delight

Code examples and unit tests from "Hacker's Delight, 2nd Edition" by Henry S. Warren Jr.

## Prerequisites

Before setting up this project, ensure you have the following installed:

```
clang --version
cmake --version
brew --version

brew install cmake ninja criterion
```

> **Note:** This project targets Apple Silicon (ARM64). The C code runs natively on M3.
> A small number of x86-specific intrinsics from the book will require ARM equivalents.

## Project Layout

```
hackers-delight/
├── CMakeLists.txt
├── src/
│   └── chapter01/
│       ├── something.c
│       └── somthing.h
└── tests/
    └── chapter01/
        └── test_something.c
```

## Building and Running Tests

From the root of the project, configure and build:

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run the unit tests with verbose output:

```
ninja && ./test_dynamic_alloc --verbose
```

Or run the test binary directly:

```
./build/test_hd --verbose
```
