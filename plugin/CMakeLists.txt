# Найдем необходимые компоненты
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)

# Попробуем найти LLVM через find_package, а если не получится, используем llvm-config
find_package(LLVM QUIET)
if(LLVM_FOUND)
    # Если LLVM найден через find_package
    
    # Добавим пути к заголовочным файлам LLVM
    include_directories(${LLVM_INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)  # Для memsafe.h в корне проекта
    
    # Добавим определения компилятора
    add_definitions(${LLVM_DEFINITIONS})
    
    # Создадим библиотеку плагина clang
    add_library(memsafe_clang SHARED
        memsafe_clang.cpp
    )
    
    # Установим свойства цели
    set_target_properties(memsafe_clang PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX ".so"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
    
    # Линкуем с LLVM библиотеками
    target_link_libraries(memsafe_clang
        ${LLVM_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
    )
    
    # Добавим include директории
    target_include_directories(memsafe_clang PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    
    # Добавим определения компилятора
    target_compile_definitions(memsafe_clang PRIVATE
        ${LLVM_DEFINITIONS}
    )
else()
    # Если LLVM не найден через find_package, используем llvm-config
    
    # Найдем llvm-config
    find_program(LLVM_CONFIG NAMES llvm-config-21 llvm-config REQUIRED)
    
    # Получим флаги из llvm-config
    execute_process(
        COMMAND ${LLVM_CONFIG} --cppflags
        OUTPUT_VARIABLE LLVM_CXXFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    execute_process(
        COMMAND ${LLVM_CONFIG} --ldflags --system-libs --libs all
        OUTPUT_VARIABLE LLVM_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Создадим библиотеку плагина clang
    add_library(memsafe_clang SHARED
        memsafe_clang.cpp
    )
    
    # Установим свойства цели
    set_target_properties(memsafe_clang PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX ".so"
        COMPILE_FLAGS "${LLVM_CXXFLAGS}"
        LINK_FLAGS "${LLVM_LDFLAGS}"
    )
    
    # Добавим include директории
    target_include_directories(memsafe_clang PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${YAML_CPP_INCLUDE_DIRS}
    )
    
    # Линкуем библиотеки
    target_link_libraries(memsafe_clang
        ${YAML_CPP_LIBRARIES}
    )
endif()

# Создадим псевдо-цель для генерации файла circleref.memsafe
add_custom_target(circleref_memsafe ALL
    COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -ferror-limit=500 -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -Xclang -load -Xclang $<TARGET_FILE:memsafe_clang>
            -Xclang -add-plugin -Xclang memsafe
            -Xclang -plugin-arg-memsafe -Xclang level=warning
            -Xclang -plugin-arg-memsafe -Xclang circleref-write=${CMAKE_CURRENT_SOURCE_DIR}/../test/circleref.memsafe
            -fsyntax-only ${CMAKE_CURRENT_SOURCE_DIR}/../test/_example.cpp
    DEPENDS memsafe_clang
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Generating circleref.memsafe"
)
