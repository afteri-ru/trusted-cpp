// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 -fsyntax-only \
// RUN: %s> %p/temp/nominal_types-CPP.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/nominal_types-CPP.cpp.out -check-prefix=CPP --allow-empty
//
// RUN: not %clangxx -I%shlibdir -std=c++20 -fsyntax-only -ferror-limit=500 \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose=config \
// RUN: %s > %p/temp/nominal_types-NOMINAL.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/nominal_types-NOMINAL.cpp.out  -check-prefix=NOMINAL

// CPP-NOT: error

// NOMINAL: Verbose mode enabled for pattern: 'config'
// NOMINAL-NOT: warning: declaration does not declare anything [-Wmissing-declarations]
// NOMINAL-NOT: fatal error
// NOMINAL-NOT: error: clang frontend command failed
// NOMINAL-NOT: error: Attribute not processed!

#include "trusted-cpp.h"

// TRUSTED_PRINT_AST("*");
typedef int IntType;
// Nominal types in static_assert not implemented!
// static_assert(std::is_same<IntType, int>::value);     // OK
// static_assert(std::is_same<IntType, IntType>::value); // OK

int int_value = 0;
IntType IntType_value = 0;
int int_value_cast = IntType_value;
IntType IntType_cast = int_value;

TRUST_NOMINAL typedef int IntSubType; // Номинальная типизация во время определения типа
// NOMINAL: verbose: Define nominal type 'IntSubType'!

// TRUSTED_PRINT_AST("*");
IntSubType IntSubType_value = 0;
int int_value_cast2 = IntSubType_value;
IntSubType IntSubType_cast = int_value;
// NOMINAL: error: The nominal type does not support automatic conversion
// NOMINAL:  IntSubType IntSubType_cast = int_value;
// NOMINAL:             ^

IntType IntType_cast2 = IntSubType_value;
// NOMINAL: error: The nominal type does not support automatic conversion
// NOMINAL:  IntType IntType_cast2 = IntSubType_value;
// NOMINAL:          ^

// Nominal types in static_assert not implemented!
// static_assert(std::is_same<IntSubType, int>::value);        // FALSE
// static_assert(std::is_same<IntSubType, IntType>::value);    // FALSE
// static_assert(std::is_same<IntSubType, IntSubType>::value); // OK

// EmptyDecl 0x7d8a7cd48c50 <nominal_types.cpp:34:31> col:31
// `-AnnotateAttr 0x7d8a7cd48c78 <trusted-cpp.h:36:33, line:68:66> "trust"
//   |-StringLiteral 0x7d8a7cd48c08 <line:64:31> 'const char[8]' lvalue "nominal"
//   `-StringLiteral 0x7d8a7cd48c28 <nominal_types.cpp:34:21> 'const char[8]' lvalue "IntType"

TRUST_NOMINAL_TYPES("IntType"); // Номинальная типизация для существующего типа

IntType IntType_value2 = 0;
IntType IntType_cast3 = int_value;
// NOMINAL: error: The nominal type does not support automatic conversion
// NOMINAL:  IntType IntType_cast3 = int_value;
// NOMINAL:          ^

IntType IntType_cast4 = IntSubType_value;
// NOMINAL: error: The nominal type does not support automatic conversion
// NOMINAL:  IntType IntType_cast4 = IntSubType_value;
// NOMINAL:          ^

// Nominal types inside operators and unsafe blocks are not implemented!
// int func(IntType int_type, int value) {
//     UNTRUSTED {
//         IntType IntType_cast5 = IntSubType_cast;
//         IntType IntType_cast6 = value;

//         return int_type + IntType_cast5 + IntType_cast6;
//     }
// }

// Nominal types in static_assert not implemented!
// static_assert(std::is_same<IntType, int>::value);     // FALSE
// static_assert(std::is_same<IntType, IntType>::value); // OK

// NOMINAL: verbose(config): plugin-config
//  ...
// NOMINAL: verbose(config): nominal-type: 'IntSubType', 'IntType'
