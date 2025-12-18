---
layout: default
title: Home
lang: en
---

# Trusted-CPP Documentation

Welcome to the English documentation for Trusted-CPP, a project that guarantees safe software development for C++ at the source code level.

## Table of Contents

- [Concept](/concept/)
- [Installation](/installation/)
- [Examples](/examples/)

## About the Project

Trusted-CPP implements the concept of safe software development based on language guarantees for C++. Our approach fixes C++'s core problems with memory and reference data types without breaking backwards compatibility with older code.

The project consists of two main components:
1. A header-only library
2. A Clang compiler plugin

These components work together to ensure memory safety in C++ applications while maintaining full compatibility with existing codebases.

## Key Features

- **Memory Safety**: Automatic memory release when there are no strong cyclic references
- **Backward Compatibility**: Works with existing C++ code without modifications
- **Thread Safety**: Automatic protection against data races in multi-threaded environments
- **Compiler Integration**: Static analysis during compilation to catch errors early

For more information, visit our [GitHub repository](https://github.com/afteri-ru/trusted-cpp).
