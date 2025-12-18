---
layout: default
title: Examples
lang: en
---

# Trusted-CPP Examples

Here are some examples demonstrating how to use Trusted-CPP in your projects.

## Basic Usage Example

To compile a file with the Trusted-CPP plugin, use the following command line:

```bash
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-disable _example.cpp
```

## Pointer Invalidation Detection

The plugin can detect invalidation of reference variables after changing data in the main variable:

```cpp
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> vec(100000, 0);
    auto x = vec.begin();
    vec = {};                // <-- Plugin will warn about using main variable 'vec'
    vec.shrink_to_fit();     // <-- Plugin will warn about using main variable 'vec'
    std::sort(x, vec.end()); // <-- Plugin will report error: Using dependent variable 'x' after changing main variable 'vec'!
    return 0;
}
```

Plugin output:
```
_example.cpp:4:17: warning: using main variable 'vec'
    vec = {};
                ^
_example.cpp:5:17: warning: using main variable 'vec'
    vec.shrink_to_fit();
                ^
_example.cpp:6:27: error: Using the dependent variable 'x' after changing the main variable 'vec'!
    std::sort(x, vec.end());
                          ^
```

## Circular Reference Detection

The plugin can also detect circular references between classes:

```cpp
class SharedCross2;  // Forward declaration

class SharedCross {
    SharedCross2 *cross2;  // <-- Plugin will detect this as part of circular reference
};

class SharedCross2 {
    SharedCross *cross;    // <-- Plugin will detect this as part of circular reference
};
```

Plugin output:
```
_cycles.cpp:53:23: error: The class 'cycles::SharedCross' has a circular reference through class 'cycles::SharedCross2'
    SharedCross2 *cross2;
                      ^
_cycles.cpp:57:22: error: The class 'cycles::SharedCross2' has a circular reference through class 'cycles::SharedCross'
    SharedCross *cross;
                     ^
```

## Library Usage Example

Using the header-only library in your code:

```cpp
#include "memsafe.h"

// Define a shared variable with thread safety
threadsafe_shared_var<std::vector<int>> myVector;

void worker_thread() {
    // Safe access to shared variable
    auto safe_access = lock(myVector);
    safe_access->push_back(42);
    // Lock automatically released at end of scope
}
```

## Compilation Options

The plugin supports several command-line arguments:

- `--circleref-disable`: Disable circular reference analysis
- `--circleref-write`: Write class information for circular reference analysis (first pass)
- `--circleref-read`: Read class information for circular reference analysis (second pass)

Example with disabled circular reference analysis:
```bash
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-disable _example.cpp
```
