---
layout: default
title: Concept
lang: en
---

# Trusted-CPP Concept

The concept of safe memory management in Trusted-CPP consists of implementing the following principles:

## Core Principles

1. **Guaranteed Memory Release**: If the program is guaranteed to have no strong cyclic references (references of an object to itself or cross-references between several objects), then when implementing the RAII principle, automatic memory release will be performed always.

2. **Type-Level Cycle Prevention**: The absence of cyclic references in the program code can only be guaranteed by prohibiting them at the level of types (class definitions).

3. **Data Race Protection**: The problem of data races when accessing memory from different threads is solved by using inter-thread synchronization objects. To prevent errors in logic, only one operator (function call) should be used to capture the synchronization object and dereference the reference at the same time in one place.

## Implementation Approach

The concept of safe memory management is ported to C++ from the NewLang language but is implemented using the standard C++ template classes `shared_ptr` and `weak_ptr`.

The main difference when working with reference variables, compared to `shared_ptr` and `weak_ptr`, is the method of dereferencing references (getting the address of an object) and the method of accessing the object. In Trusted-CPP, access can be done not only by dereferencing the reference "*", but also by capturing (locking) the reference and storing it in a temporary variable. The lifetime of this variable is limited and automatically controlled by the compiler, and through it direct access to the data itself (the object) is carried out.

Such an automatic variable is a temporary strong reference holder and is similar to `std::lock_guard` - a synchronization object holder until the end of the current scope (lifetime), after which it is automatically deleted by the compiler.

## Memory Safety Through Static Analysis

Our Clang plugin performs static analysis of C++ code during compilation. It checks for invalidation of reference types (iterators, `std::span`, `std::string_view`, etc.) when data in the original variable is changed and controls strong cyclic references at the type level (class definitions) of any nesting.

## Thread Safety Mechanisms

The library provides automatic data race protection when accessing shared variables from different threads. The access control method must be specified when defining the variable, after which the acquisition and release of the synchronization object will occur automatically when the reference is dereferenced.

By default, shared variables are created without multi-thread access control and have no additional overhead compared to the standard template classes `std::shared_ptr` and `std::weak_ptr`.

## Backward Compatibility

One of the key aspects of Trusted-CPP is maintaining full backward compatibility with existing C++ code. The system works with C++20 but can be adapted to work with C++17 or even C++11 with minimal modifications.
