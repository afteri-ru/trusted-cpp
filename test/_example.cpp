// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 -c %s -o %p/temp/_example_clear.o
//
// The plugin finds errors and the code doesn't compile.
//
// RUN: not %clangxx -I%shlibdir -std=c++20 -ferror-limit=500 \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose \
// RUN: -Xclang -plugin-arg-trust -Xclang circleref-disable \
// RUN: -c %s -o %p/temp/_example.o > %p/temp/_example.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/_example.cpp.out

// CHECK: Verbose mode enabled
// CHECK: Circular reference analysis disabled
// CHECK-NOT: fatal error

#include <algorithm>
#include <vector>

#include "trusted-cpp.h"

using namespace trust;

namespace ns {

void *invalidate_test(int arg) {

    std::vector<int> vect{1, 2, 3, 4};
    {

        auto beg = vect.begin();
        // CHECK: verbose: Var found beg:auto-type
        // CHECK-NEXT: verbose: beg:auto-type=>vect

        std::vector<int>::const_iterator beg_const = vect.cbegin();
        // CHECK: verbose: Var found beg_const:auto-type
        // CHECK-NEXT: verbose: beg_const:auto-type=>vect
        // CHECK-NEXT: verbose: Only constant method 'cbegin' does not change data.

        auto pointer = &vect; // Error
        // CHECK: error: Raw address
        // CHECK-NEXT:       auto pointer = &vect; // Error
        // CHECK-NEXT:            ^

        auto str = std::string();
        std::string_view view(str);
        // CHECK: verbose: Var found view:auto-type
        // CHECK-NEXT: verbose: view:auto-type=>str

        str.clear();
        // CHECK-NEXT: verbose: Only non constant method 'clear' alway changed data.

        auto view_iter = view.begin(); // Error
        // CHECK-NEXT: error: Raw address
        // CHECK-NEXT:      auto view_iter = view.begin(); // Error
        // CHECK-NEXT:           ^
        // CHECK-NEXT: verbose: view_iter:raw-addr=>view

        // CHECK-NEXT: warning: using main variable 'str'
        // CHECK-NEXT:       str.clear();
        // CHECK-NEXT:       ^
        // CHECK-NEXT: error: Using the dependent variable 'view' after changing the main variable 'str'!
        // CHECK-NEXT:       auto view_iter = view.begin(); // Error
        // CHECK-NEXT:                        ^

        {
            vect = {};
            // CHECK: verbose: Only non constant method 'std::vector<int>::operator=' alway changed data.

            vect.shrink_to_fit();
            // CHECK-NEXT: verbose: Only non constant method 'shrink_to_fit' alway changed data.
            // CHECK-NEXT: warning: using main variable 'vect'
            // CHECK-NEXT:            vect = {};
            // CHECK-NEXT:            ^
            // CHECK-NEXT: warning: using main variable 'vect'
            // CHECK-NEXT:            vect.shrink_to_fit();
            // CHECK-NEXT:            ^

            std::sort(beg, vect.end()); // Error
            // CHECK: error: Using the dependent variable 'beg' after changing the main variable 'vect'!
            // CHECK-NEXT:           std::sort(beg, vect.end()); // Error
            // CHECK-NEXT:                    ^

            const std::vector<int> vect_const;
            // CHECK: verbose: Both methods 'end' for constant and non-constant objects tracking enabled!
        }
    }

    {

        auto b = LazyCaller<decltype(vect), decltype(std::declval<decltype(vect)>().begin())>(vect, &decltype(vect)::begin);
        // CHECK: verbose: Inplace address arithmetic
        auto e = LAZYCALL(vect, end);
        // CHECK-NEXT: verbose: Inplace address arithmetic
        {
            vect = {};
            vect.clear();
            std::sort(*b, *e);
        }
    }

    UNTRUSTED {
        // CHECK: verbose: Unsafe statement
        return nullptr;
        UNTRUSTED return nullptr;
        // CHECK-NEXT: verbose: Unsafe statement
    }
}

namespace UNTRUSTED {

Shared<int> var_unsafe1(1);
// CHECK: verbose: Var found var_unsafe1:shared-type

trust::Shared<int> var_unsafe2(2);
// CHECK-NEXT: verbose: Var found var_unsafe2:shared-type

trust::Shared<int> var_unsafe3(3);
// CHECK-NEXT: verbose: Var found var_unsafe3:shared-type

} // namespace UNTRUSTED

trust::Value<int> var_value(1);

trust::Value<int> var_value2(2);

trust::Shared<int> var_share(1);
// CHECK-NEXT: verbose: Var found var_share:shared-type

trust::Shared<int, trust::SyncTimedMutex> var_guard(1);
// CHECK-NEXT: verbose: Var found var_guard:shared-type

static trust::Value<int> var_static(1);
static auto static_fail1(var_static.lock()); // Error
// CHECK-NEXT: error: Create auto variabe as static static_fail1:auto-type
// CHECK-NEXT:  static auto static_fail1(var_static.lock()); // Error
// CHECK-NEXT:              ^
// CHECK-NEXT: verbose: static_fail1:auto-type=>var_static

static auto static_fail2 = var_static.lock(); // Error
// CHECK-NEXT: error: Create auto variabe as static static_fail2:auto-type
// CHECK-NEXT:  static auto static_fail2 = var_static.lock(); // Error
// CHECK-NEXT:              ^
// CHECK-NEXT: verbose: static_fail2:auto-type=>var_static
// CHECK-NEXT: verbose: Both methods 'lock' for constant and non-constant objects tracking enabled!

trust::Shared<int> memory_test(trust::Shared<int> arg, trust::Value<int> arg_val) {

    var_static = var_value;
    // CHECK-NEXT: verbose: Only non constant method 'trust::Value<int>::operator=' alway changed data.

    {
        var_static = var_value;
        // CHECK-NEXT: verbose: Only non constant method 'trust::Value<int>::operator=' alway changed data.
        {
            var_static = var_value;
            // CHECK-NEXT: verbose: Only non constant method 'trust::Value<int>::operator=' alway changed data.
        }
    }

    trust::Shared<int> var_shared1(1);
    // CHECK-NEXT: verbose: Var found var_shared1:shared-type

    trust::Shared<int> var_shared2(1);
    // CHECK-NEXT: verbose: Var found var_shared2:shared-type

    var_shared1 = var_shared1;           // ?????????????????
    UNTRUSTED var_shared1 = var_shared2; // Unsafe
    // CHECK-NEXT: verbose: Unsafe statement

    {
        trust::Shared<int> var_shared3(3);
        // CHECK-NEXT: verbose: Var found var_shared3:shared-type

        var_shared1 = var_shared1; // ?????????????
        var_shared2 = var_shared1; // ????????????
        var_shared3 = var_shared1;

        {
            trust::Shared<int> var_shared4 = var_shared1;
            // CHECK-NEXT: verbose: Var found var_shared4:shared-type

            var_shared1 = var_shared1; // ????????????????
            var_shared2 = var_shared1; // ???????????????
            var_shared3 = var_shared1;

            var_shared4 = var_shared1;
            var_shared4 = var_shared3;

            var_shared4 = var_shared4; // ????????????????????

            if (var_shared4) {
                UNTRUSTED return var_shared4;
                // CHECK-NEXT: verbose: Unsafe statement
            }
            return var_shared4; // ????????????????
        }

        std::swap(var_shared1, var_shared2);
        std::swap(var_shared1, var_shared3);
        UNTRUSTED std::swap(var_shared1, var_shared3);
        // CHECK-NEXT: verbose: Unsafe statement

        return arg; // ?????????????????
    }

    int temp = 3;
    temp = 4;
    var_value = 5;
    *var_value += 6;

    return 777;
}

void shared_example() {
    std::shared_ptr<int> old_shared; // Error
    // CHECK: error: Error type found 'std::shared_ptr'
    // CHECK-NEXT:    std::shared_ptr<int> old_shared; // Error
    // CHECK-NEXT:                         ^
    // CHECK-NEXT: verbose: Var found old_shared:shared-type

    Shared<int> var = 1;
    // CHECK-NEXT: verbose: Var found var:shared-type

    Shared<int> copy;
    // CHECK-NEXT: verbose: Var found copy:shared-type

    copy = var; // ???????????????
    std::swap(var, copy);
    {
        Shared<int> inner = var;
        // CHECK-NEXT: verbose: Var found inner:shared-type

        std::swap(inner, copy); // ?????????????
        inner = copy;
        copy = inner; // ????????????????????
    }
}

trust::Shared<int> memory_test_8(trust::Shared<int> arg) {
    return arg; // ????????????????
}

trust::Shared<int> memory_test_9() { return Shared<int>(999); }

struct Ext;

struct A {
    Shared<Ext> ext;
};

void bugfix_11() { // https://github.com/rsashka/trust/issues/11
    std::vector vect(100000, 0);
    auto x = (vect.begin());
    // CHECK: verbose: Var found x:auto-type
    // CHECK-NEXT: verbose: x:auto-type=>vect
    // CHECK-NEXT: verbose: Only non constant method 'std::vector<int>::operator=' alway changed data.

    vect = {};
    // CHECK-NEXT: warning: using main variable 'vect'
    // CHECK-NEXT:     vect = {};
    // CHECK-NEXT:     ^

    std::sort(x, vect.end()); // Error
    // CHECK-NEXT: error: Using the dependent variable 'x' after changing the main variable 'vect'!
    // CHECK-NEXT:     std::sort(x, vect.end()); // Error
    // CHECK-NEXT:               ^
    // CHECK-NEXT: verbose: Both methods 'end' for constant and non-constant objects tracking enabled!
}

void bugfix_12() { // https://github.com/rsashka/trust/issues/12
    std::vector vect(100000, 0);
    auto &y = vect[0];
    // CHECK: verbose: y:raw-addr=>vect

    vect = {};
    // CHECK-NEXT: verbose: Only non constant method 'std::vector<int>::operator=' alway changed data.
    // CHECK-NEXT: warning: using main variable 'vect'
    // CHECK-NEXT:      vect = {};
    // CHECK-NEXT:      ^

    y += 1; // Error
    // CHECK-NEXT:error: Using the dependent variable 'y' after changing the main variable 'vect'!
    // CHECK-NEXT:    y += 1; // Error
    // CHECK-NEXT:    ^
}
} // namespace ns

// CHECK: verbose: plugin-config
// CHECK-NEXT: verbose: error-type: 'std::auto_ptr', 'std::shared_ptr'
// CHECK-NEXT: verbose: warning-type: 'std::auto_ptr', 'std::shared_ptr'
// CHECK-NEXT: verbose: auto-type: '__gnu_cxx::__normal_iterator', 'std::basic_string_view', 'std::reverse_iterator', 'std::span', 'trust::Locker'
// CHECK-NEXT: verbose: shared-type: 'std::shared_ptr', 'trust::Shared'
// CHECK-NEXT: verbose: not-shared-classes: ''
// CHECK-NEXT: verbose: invalidate-func: 'std::move', 'std::swap'
