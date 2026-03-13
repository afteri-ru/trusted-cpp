//
// Prepare
// if [ -f "%p/temp/circleref.trust" ]; then rm %p/temp/circleref.trust; fi
//
// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 %s -fsyntax-only
//
// The plugin finds errors and the code doesn't compile.
//
// RUN: not %clangxx -I%shlibdir -std=c++20 -ferror-limit=500 -fsyntax-only \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose \
// RUN: -Xclang -plugin-arg-trust -Xclang circleref-disable \
// RUN: %s > %p/temp/using_globals.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/using_globals.cpp.out

// CHECK: Verbose mode enabled
// CHECK-NOT: warning: declaration does not declare anything [-Wmissing-declarations]
// CHECK-NOT: fatal error
// CHECK-NOT: error: clang frontend command failed
// CHECK-NOT: error: Attribute not processed!

#include "trusted-cpp.h"

// TRUSTED_PRINT_AST("*");

int global = 0;
namespace ns {
int global_1 = 0;
namespace ns2 {
const int global_2 = 0;
static const int global_3 = 0;
} // namespace ns2

} // namespace ns

constexpr int global_4 = 0;

using namespace ns;
using namespace ns::ns2;

int func_open() {
    // open scope and free access to global variables
    return global + global_1 + global_2 + global_3 + global_4;
}

namespace ns {
TRUST_USING_EXTERNAL("") // Deny access to all global variables
int func_closed() {
    // CHECK: verbose: Using globals ''

    // Global variables cannot be accessed from a closed scope
    return global + global_1 + global_2 + global_3 + global_4;

    // CHECK: error: Variable name 'global' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:         ^
    // CHECK: error: Variable name 'global_1' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                   ^
    // CHECK: error: Variable name 'global_2' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                             ^
    // CHECK: error: Variable name 'global_3' does not match patterns!!
    // CHECK-NEXT:   return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                                         ^
    // CHECK: error: Variable name 'global_4' does not match patterns!!
    // CHECK-NEXT:   return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                                                    ^
}
}; // namespace ns

TRUST_USING_EXTERNAL("*global_*") // Except for the "global" variable
int func_closed_mask_part() {
    // CHECK: verbose: Using globals '*global_*'

    // Global variables cannot be accessed from a closed scope
    return global + global_1 + global_2 + global_3 + global_4;

    // CHECK: error: Variable name 'global' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:         ^
    // CHECK: verbose: Variable name 'global_1' matches pattern!
    // CHECK: verbose: Variable name 'global_2' matches pattern!
    // CHECK: verbose: Variable name 'global_3' matches pattern!
    // CHECK: verbose: Variable name 'global_4' matches pattern!
}

TRUST_USING_EXTERNAL("ns*") // Except for the "ns" namespace, including nested
int func_closed_mask_part2() {
    // CHECK: verbose: Using globals 'ns*'

    TRUST_USING_EXTERNAL("global") {
        // CHECK: verbose: Using globals 'global'

        return global + global_1 + global_2 + global_3;

        // CHECK: verbose: Variable name 'global' matches pattern!
        // CHECK: verbose: Variable name 'global_1' matches pattern!
        // CHECK: verbose: Variable name 'global_2' matches pattern!
        // CHECK: verbose: Variable name 'global_3' matches pattern!
    }

    // Global variables cannot be accessed from a closed scope
    return global_1 + global_2 + global_3 + global_4;

    // CHECK: verbose: Variable name 'global_1' matches pattern!
    // CHECK: verbose: Variable name 'global_2' matches pattern!
    // CHECK: verbose: Variable name 'global_3' matches pattern!
    // CHECK: error: Variable name 'global_4' does not match patterns!!
    // CHECK:   return global_1 + global_2 + global_3 + global_4;
    // CHECK:                                           ^
}

TRUST_USING_EXTERNAL("ns:global_1;*global_2") // Access only for global_1 and global_2
int func_closed_mask_list() {
    // CHECK: verbose: Using globals 'ns:global_1;*global_2'

    // Global variables cannot be accessed from a closed scope
    return global + global_1 + global_2 + global_3 + global_4;
    // CHECK: error: Variable name 'global' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:         ^
    // CHECK: error: Variable name 'global_1' does not match patterns!!
    // CHECK-NEXT:   return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                   ^
    // CHECK: verbose: Variable name 'global_2' matches pattern!
    // CHECK: error: Variable name 'global_3' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                             ^
    // CHECK: error: Variable name 'global_4' does not match patterns!!
    // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
    // CHECK-NEXT:                                                   ^
}

TRUST_USING_EXTERNAL("*") // Access for any global variables
int func_closed_mask_all() {
    // CHECK: verbose: Using globals '*'
    // CHECK: verbose: Variable name 'global' matches pattern!
    // CHECK: verbose: Variable name 'global_1' matches pattern!
    // CHECK: verbose: Variable name 'global_2' matches pattern!
    // CHECK: verbose: Variable name 'global_3' matches pattern!
    // CHECK: verbose: Variable name 'global_4' matches pattern!

    // Global variables cannot be accessed from a closed scope
    return global + global_1 + global_2 + global_3 + global_4;
}

class ClassOpen {
  public:
    int OpenMethod() {
        // open scope and free access to global variables
        return global + global_1 + global_2 + global_3 + global_4;
    }

    TRUST_USING_EXTERNAL("")
    int ClosedMethod() {
        // CHECK: verbose: Using globals ''

        // Global variables cannot be accessed from a closed scope
        return global + global_1 + global_2 + global_3 + global_4;

        // CHECK: error: Variable name 'global' does not match patterns!!
        // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
        // CHECK-NEXT:         ^
        // CHECK: error: Variable name 'global_1' does not match patterns!!
        // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
        // CHECK-NEXT:                   ^
        // CHECK: error: Variable name 'global_2' does not match patterns!!
        // CHECK-NEXT:  return global + global_1 + global_2 + global_3 + global_4;
        // CHECK-NEXT:                             ^
        // CHECK: error: Variable name 'global_3' does not match patterns!!
        // CHECK-NEXT:   return global + global_1 + global_2 + global_3 + global_4;
        // CHECK-NEXT:                                         ^
        // CHECK: error: Variable name 'global_4' does not match patterns!!
        // CHECK-NEXT:   return global + global_1 + global_2 + global_3 + global_4;
        // CHECK-NEXT:                                                    ^
    }
};

struct TRUST_USING_EXTERNAL("") // Make all class methods have closed scope and do not have access to global variables.
    ClassClosed {
    // CHECK: verbose: Set restriction name '' for ClassClosed

    int field;

    int ClosedMethod() {
        // Global variables cannot be accessed from a closed scope
        return global + global_1 + global_2 + global_3 + global_4;
    }
    TRUST_USING_EXTERNAL("*") // open scope and free access to global variables
    int OpenMethod() {
        // CHECK: verbose: Using globals '*'

        return global + global_1 + global_2 + global_3 + global_4;
    }
    int Other1();

    TRUST_USING_EXTERNAL("*")
    int Other2();
};

int ClassClosed::Other1() { return global + global_1 + global_2 + global_3 + global_4; }
int ClassClosed::Other2() { return global + global_1 + global_2 + global_3 + global_4; }

int func_classes() {
    ClassOpen open;
    ClassClosed closed;

    return open.OpenMethod() + open.ClosedMethod() + closed.OpenMethod() + closed.ClosedMethod() + closed.Other1() + closed.Other2();
}