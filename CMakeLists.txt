cmake_minimum_required(VERSION 3.20)

# Основные настройки проекта
project(trust-cpp VERSION 1.0.0 LANGUAGES C CXX)

# Используем clang++ напрямую, как в исходных Makefile
set(CMAKE_CXX_COMPILER clang++-21)

# Установим стандарт C++20 и флаги оптимизации
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Поиск необходимых компонентов
find_package(LLVM REQUIRED CONFIG)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)

# Общие настройки целей
function(set_common_properties TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endfunction()

# Создание библиотеки плагина clang
add_library(trusted-cpp_clang SHARED
    plugin/trusted-cpp_clang.cpp
)

# Настройка плагина
target_include_directories(trusted-cpp_clang PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${YAML_CPP_INCLUDE_DIRS}
)

target_compile_definitions(trusted-cpp_clang PRIVATE ${LLVM_DEFINITIONS})

set_target_properties(trusted-cpp_clang PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_common_properties(trusted-cpp_clang)

target_link_libraries(trusted-cpp_clang
    LLVMCore
    LLVMSupport
    LLVMAnalysis
    LLVMTransformUtils
    ${YAML_CPP_LIBRARIES}
)

# Настройка целей тестирования
add_executable(trusted-cpp_unittest
    test/trusted-cpp_test.cpp
)

set_target_properties(trusted-cpp_unittest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/temp
)

set_common_properties(trusted-cpp_unittest)

target_compile_definitions(trusted-cpp_unittest PRIVATE
    BUILD_UNITTEST
)

target_include_directories(trusted-cpp_unittest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin
    ${YAML_CPP_INCLUDE_DIRS}
)

target_link_libraries(trusted-cpp_unittest 
    GTest::GTest 
    GTest::Main 
    yaml-cpp
    pthread
)

# Обеспечение зависимостей
add_dependencies(trusted-cpp_unittest trusted-cpp_clang)

# Цель для запуска тестов
add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/temp/trusted-cpp_unittest
    DEPENDS trusted-cpp_unittest

    COMMAND echo Run: LLVM Integrated Tester in ${CMAKE_CURRENT_SOURCE_DIR}/test
    COMMAND ${PYTHON_EXECUTABLE} /usr/lib/llvm-21/build/utils/lit/lit.py ${CMAKE_CURRENT_SOURCE_DIR}/test -v

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
    COMMENT "Running trust unit tests"
)
