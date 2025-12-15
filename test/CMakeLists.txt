# Проверим наличие необходимых компонентов
find_package(GTest REQUIRED)

# Создадим цель для тестов
add_executable(memsafe_unittest
    _cycles.cpp
    _example.cpp
    memsafe_test.cpp
)

# Установим свойства цели для тестов
set_target_properties(memsafe_unittest PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Добавим определения для тестов
target_compile_definitions(memsafe_unittest PRIVATE
    BUILD_UNITTEST
)

# Добавим пути к заголовочным файлам
target_include_directories(memsafe_unittest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin
    ${YAML_CPP_INCLUDE_DIRS}
)

# Добавим линковку библиотек
target_link_libraries(memsafe_unittest 
    GTest::GTest 
    GTest::Main 
    yaml-cpp
    pthread
)

# Добавим пользовательские команды для анализа _cycles.cpp и _example.cpp с использованием импортированной цели
# Для _cycles.cpp
add_custom_command(TARGET memsafe_unittest PRE_BUILD
    COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -g -DBUILD_UNITTEST -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -ferror-limit=500 
            "-Xclang" "-load" "-Xclang" "${CMAKE_CURRENT_SOURCE_DIR}/../memsafe_clang.so"
            "-Xclang" "-add-plugin" "-Xclang" "memsafe"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "log"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "level=warning"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "circleref-read=${CMAKE_CURRENT_SOURCE_DIR}/circleref.memsafe"
            -MMD -MP -MF ${CMAKE_CURRENT_BINARY_DIR}/_cycles.d
            -o ${CMAKE_CURRENT_BINARY_DIR}/_cycles.o
            -c ${CMAKE_CURRENT_SOURCE_DIR}/_cycles.cpp
    DEPENDS memsafe_clang circleref_memsafe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analyzing _cycles.cpp with memsafe plugin"
)

# Для _example.cpp
add_custom_command(TARGET memsafe_unittest PRE_BUILD
    COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -g -DBUILD_UNITTEST -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -ferror-limit=500 
            "-Xclang" "-load" "-Xclang" "${CMAKE_CURRENT_SOURCE_DIR}/../memsafe_clang.so"
            "-Xclang" "-add-plugin" "-Xclang" "memsafe"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "log"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "level=warning"
            "-Xclang" "-plugin-arg-memsafe" "-Xclang" "circleref-disable"
            -MMD -MP -MF ${CMAKE_CURRENT_BINARY_DIR}/_example.d
            -o ${CMAKE_CURRENT_BINARY_DIR}/_example.o
            -c ${CMAKE_CURRENT_SOURCE_DIR}/_example.cpp
    DEPENDS memsafe_clang
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Analyzing _example.cpp with memsafe plugin"
)

# Создадим цель для запуска тестов
add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/memsafe_unittest
    DEPENDS memsafe_unittest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running memsafe unit tests"
)
