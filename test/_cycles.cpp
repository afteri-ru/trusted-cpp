//
// Prepare
// RUN: if [ -f "%p/temp/circleref.trust" ]; then rm %p/temp/circleref.trust; fi
//
// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 -c %s -o %p/temp/_circle_clear.o
//
// The plugin finds errors and the code doesn't compile.
//
// RUN: %clangxx -I%shlibdir -std=c++20 -ferror-limit=500 \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose \
// RUN: -Xclang -plugin-arg-trust -Xclang circleref-write=%p/temp/circleref.trust \
// RUN: -c %s -o %p/temp/_circle.o > %p/temp/_circle.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/_circle.cpp.out

// CHECK: Verbose mode enabled
// CHECK: Write the circular reference analysis data to file {{.*}}/temp/circleref.trust
// CHECK-NOT: fatal error

#include "trusted-cpp.h"
#include <algorithm>
#include <map>
#include <vector>

using namespace trust;

namespace ns {
struct A;

// cyclic cross-references between classes that are defined in other translation units
//    struct A {
//        Shared<Ext> ext;
//    };

class Ext {
    Shared<A> a;
};

} // namespace ns

// cyclic-analyzer
namespace cycles {

// simple circular self-references

struct CircleSelf {
    CircleSelf *self;
    // CHECK: verbose: Field with reference to structured data type 'cycles::CircleSelf'
};

struct CircleShared {
    Shared<CircleShared> shared;
};

struct CircleSelfUnsafe {
    UNTRUSTED CircleSelf *self;
    // CHECK: verbose: Field with reference to structured data type 'cycles::CircleSelf'
};

struct CircleSharedUnsafe {
    UNTRUSTED Shared<CircleShared> shared;
};

// cyclic cross-references

class SharedCross2;

class SharedCross {
    SharedCross2 *cross2;
};

class SharedCross2 {
    SharedCross *cross;
    // CHECK: verbose: Field with reference to structured data type 'cycles::SharedCross'
    // CHECK: verbose: Field with reference to structured data type 'cycles::SharedCross'
};

class SharedCrossUnsafe {
    UNTRUSTED SharedCross2 *cross2;
    // CHECK: verbose: Field with reference to structured data type 'cycles::SharedCross2'
    // CHECK: verbose: Field with reference to structured data type 'cycles::SharedCross2'
};

class SharedCross2Unsafe {
    UNTRUSTED SharedCross *cross;
    // CHECK: verbose: Field with reference to structured data type 'cycles::SharedCross'
};

// cyclic cross-references between classes that are defined in other translation units

struct ExtExt : public ns::Ext {};

class ExtExtExt : public ExtExt {};

// Reference types in STD templates

struct ArraySharedInt : public std::vector<Shared<int>> {};

struct SharedArrayInt : public Shared<std::vector<int>> {};

// Reference types when extending templates

template <typename T> class SharedSingle : public Shared<T, SyncSingleThread> {};

class SharedSingleInt : public SharedSingle<int> {};

class SharedSingleIntField {
    SharedSingleInt shared_int;
};

// Non-reference types with weak references
class NotShared1 {
    int interger;
    std::weak_ptr<NotShared1> weak;
};

struct NotShared2 : public NotShared1 {
    NotShared1 not1;
    std::weak_ptr<NotShared2> weak1;
    Weak<Shared<int>> weak2;
};

} // namespace cycles

// CHECK: Write file '{{.*}}/temp/circleref.trust' complete!
