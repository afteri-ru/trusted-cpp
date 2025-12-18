---
layout: default
title: Installation
lang: en
---

# Installation Guide

This guide will help you install and set up Trusted-CPP on your system.

## Prerequisites

Before installing Trusted-CPP, ensure you have the following:

- A C++ compiler that supports C++20 (clang++ 20 or above recommended)
- CMake 3.12 or higher
- Git (for cloning the repository)

## Building from Source

1. Clone the repository:
```bash
git clone https://github.com/afteri-ru/trusted-cpp.git
cd trusted-cpp
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Configure the project with CMake:
```bash
cmake ..
```

4. Build the project:
```bash
make
```

This will build both the Clang plugin (`memsafe_clang.so`) and the test executable.

## Using the Plugin

After building, you can use the plugin with clang++ to analyze your C++ code:

```bash
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-disable your_file.cpp
```

Replace `your_file.cpp` with the path to your C++ source file.

## Plugin Options

The plugin supports several command-line arguments:

- `--circleref-disable`: Disable circular reference analysis (default)
- `--circleref-write`: Write class information for circular reference analysis (first pass)
- `--circleref-read`: Read class information for circular reference analysis (second pass)

For circular reference analysis, you'll need to run the plugin twice:
1. First pass with `--circleref-write` to collect class information
2. Second pass with `--circleref-read` to perform the analysis

Example for circular reference analysis:
```bash
# First pass
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-write -fsyntax-only your_file.cpp

# Second pass
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-read your_file.cpp
```

## Using the Library

Include the `memsafe.h` header in your C++ files to use the library features:

```cpp
#include "memsafe.h"

// Your code here
```

The header-only library provides enhanced smart pointer types and thread safety mechanisms.

## Running Tests

To run the tests, build the test executable and run it:

```bash
./memsafe_test
```

This will execute all the unit tests for the Trusted-CPP library and plugin.
