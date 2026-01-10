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

# Упрощенная функция для добавления команд анализа с плагином
function(add_analysis_command TARGET_NAME SOURCE_FILE OUTPUT_FILE)
    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -ferror-limit=500 -g -DBUILD_UNITTEST -I${CMAKE_CURRENT_SOURCE_DIR}
                -Xclang -load -Xclang $<TARGET_FILE:trusted-cpp_clang>
                -Xclang -add-plugin -Xclang trust
                -Xclang -plugin-arg-trust -Xclang log
                -Xclang -plugin-arg-trust -Xclang level=warning
                ${ARGN}
                -MMD -MP -MF ${OUTPUT_FILE}.d
                -o ${OUTPUT_FILE}.o
                -c ${SOURCE_FILE}
        DEPENDS ${TARGET_NAME} trusted-cpp_clang
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Analyzing ${SOURCE_FILE} with trust plugin"
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

# Генерация файла circleref.trust
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/circleref.trust
    COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -ferror-limit=500 -I${CMAKE_CURRENT_SOURCE_DIR}/test
            -Xclang -load -Xclang $<TARGET_FILE:trusted-cpp_clang>
            -Xclang -add-plugin -Xclang trust
            -Xclang -plugin-arg-trust -Xclang level=warning
            -Xclang -plugin-arg-trust -Xclang circleref-write=${CMAKE_CURRENT_SOURCE_DIR}/test/circleref.trust
            -fsyntax-only ${CMAKE_CURRENT_SOURCE_DIR}/test/_example.cpp
    DEPENDS trusted-cpp_clang
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating circleref.trust"
)

add_custom_target(circleref_trust ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/circleref.trust
)

# Настройка целей тестирования
add_executable(trusted-cpp_unittest
    test/_cycles.cpp
    test/_example.cpp
    test/trusted-cpp_test.cpp
)

set_target_properties(trusted-cpp_unittest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
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

# Добавление команд анализа для тестовых файлов
add_analysis_command(trusted-cpp_unittest
    ${CMAKE_CURRENT_SOURCE_DIR}/test/_cycles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/_cycles
    -Xclang -plugin-arg-trust -Xclang circleref-read=${CMAKE_CURRENT_SOURCE_DIR}/test/circleref.trust
)

add_analysis_command(trusted-cpp_unittest
    ${CMAKE_CURRENT_SOURCE_DIR}/test/_example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/_example
    -Xclang -plugin-arg-trust -Xclang circleref-disable
)

# Цель для запуска тестов
add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/trusted-cpp_unittest
    DEPENDS trusted-cpp_unittest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
    COMMENT "Running trust unit tests"
)
