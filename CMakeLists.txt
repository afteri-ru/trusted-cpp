cmake_minimum_required(VERSION 3.20)

project(trust-cpp VERSION 1.0.0 LANGUAGES C CXX)

# Используем clang++ напрямую, как в исходных Makefile
set(CMAKE_CXX_COMPILER clang++-21)

# Установим стандарт C++20, как в оригинальном Makefile
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавим флаги оптимизации
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Найдем необходимые компоненты
find_package(LLVM REQUIRED CONFIG)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)

# Определим функцию для установки общих свойств целей
function(set_common_target_properties TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endfunction()


# Определим функцию для анализа файлов с использованием плагина
function(add_trust_analysis_command SOURCE_FILE OUTPUT_FILE DEPENDS_TARGET ARG1 ARG2)
    add_custom_command(TARGET ${DEPENDS_TARGET} PRE_BUILD
        COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -ferror-limit=500 -g -DBUILD_UNITTEST -I${CMAKE_CURRENT_SOURCE_DIR}
                -Xclang -load -Xclang "${CMAKE_SOURCE_DIR}/trusted-cpp_clang.so"
                -Xclang -add-plugin -Xclang trust
                -Xclang -plugin-arg-trust -Xclang log
                -Xclang -plugin-arg-trust -Xclang level=warning
                ${ARG1} ${ARG2}
                -MMD -MP -MF ${OUTPUT_FILE}.d
                -o ${OUTPUT_FILE}.o
                -c ${SOURCE_FILE}
        DEPENDS ${DEPENDS_TARGET} ${DEPENDS_TARGET}_trust
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Analyzing ${SOURCE_FILE} with trust plugin"
    )
endfunction()

# Добавим подпроекты
add_subdirectory(plugin)
add_subdirectory(test)

# Добавим команду для локального тестирования сайта
add_custom_target(serve-docs
    COMMAND bundle exec jekyll serve --source docs --destination docs/_site --incremental --host 127.0.0.1 --port 4000
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Запуск локального сервера Jekyll для документации"
)
