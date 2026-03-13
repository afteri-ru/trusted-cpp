// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 -fsyntax-only %s
//
// The plugin finds errors and the code doesn't compile.
//
// RUN: CLANG_TRUST="%clangxx" not %p/../trusted-cpp.sh -std=c++20 -fsyntax-only %s > %p/temp/invalidate.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/invalidate.cpp.out

#include <algorithm>
#include <vector>

#include "trusted-cpp.h"

using namespace trust;

int main() {
    std::vector vect(100000, 0);
    auto x = vect.begin();
    auto &y = vect[0];

    vect = {};
    // CHECK: warning: using main variable 'vect'
    // CHECK-NEXT:     vect = {};
    // CHECK-NEXT:     ^

    std::sort(x, vect.end()); // Error
    // CHECK: error: Using the dependent variable 'x' after changing the main variable 'vect'!
    // CHECK-NEXT:     std::sort(x, vect.end()); // Error
    // CHECK-NEXT:               ^

    y += 1; // Error
    // CHECK:error: Using the dependent variable 'y' after changing the main variable 'vect'!
    // CHECK-NEXT:    y += 1; // Error
    // CHECK-NEXT:    ^

    return 0;
}
