//
// RUN: %clangxx -I%shlibdir -std=c++20 -ferror-limit=500 -fsyntax-only \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose=lifetime \
// RUN: -Xclang -plugin-arg-trust -Xclang circleref-disable \
// RUN: %s > %p/temp/lifetime.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/lifetime.cpp.out

// CHECK: Verbose mode enabled for pattern: 'lifetime'
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

int func_first(int arg) {
    // CHECK: verbose(lifetime): start 1: func_first
    // CHECK: verbose(lifetime): start 2:
    int local = 0;
    if (int var = (global + arg)) {
        // CHECK: verbose(lifetime): start 3:
        if (var) {
            // CHECK: verbose(lifetime): start 4:
            return global_3;
        }
        // CHECK: verbose(lifetime): end 4:
        return local;
    } else {
        // CHECK: verbose(lifetime): end 3:
        // CHECK: verbose(lifetime): start 3:
        return global_3 + global_4;
    }
    // CHECK: verbose(lifetime): end 3:
    {
        // CHECK: verbose(lifetime): start 3:
        int local3 = arg;
        {
            // CHECK: verbose(lifetime): start 4:
            int local4 = arg;
            while (arg--) {
                // CHECK: verbose(lifetime): start 5:
                if (arg < 0) {
                    // CHECK: verbose(lifetime): start 6:
                    return arg + local4;
                }
                // CHECK: verbose(lifetime): end 6:
            }
            // CHECK: verbose(lifetime): end 5:
        }
        // CHECK: verbose(lifetime): end 4:
    }
    // CHECK: verbose(lifetime): end 3:
    return arg;
}
// CHECK: verbose(lifetime): end 2:
// CHECK: verbose(lifetime): end 1: func_first

namespace ns {
int func_second(int arg1, int arg2) { 
    // CHECK: verbose(lifetime): start 1: ns::func_second
    // CHECK: verbose(lifetime): start 2:
    return global + arg1 + arg2; 
}
// CHECK: verbose(lifetime): end 2:
// CHECK: verbose(lifetime): end 1: ns::func_second

class ClassNS {
  public:
    int field;
    static int MethodStatic() { return global; }

    int Method(int arg) { return global + global_1 + arg; }
    int Method2(int arg);
};

}; // namespace ns

class ClassNext {
  public:
    int field;
    static constexpr int field_static = 0;
    static int MethodStatic() { return global + field_static; }

    int Method(int arg);
};

int ns::ClassNS::Method2(int arg) { return global + global_1 + arg + field; }
int ClassNext::Method(int arg) { return global + global_1 + arg + field_static + field; }

int func_classes(int arg) {
    ClassNS cls_ns;
    ClassNext next;

    return cls_ns.Method(arg) + cls_ns.MethodStatic() + next.field + next.Method(arg);
}