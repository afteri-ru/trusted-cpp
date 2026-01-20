import lit.formats
import os

# Основные настройки проекта
config.name = "Stack Overflow Check"
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.test_source_root, "temp")
config.test_format = lit.formats.ShTest(not lit_config.useValgrind)

# Настройки тестов
config.suffixes = ['.c', '.cpp']
config.excludes = ['trusted-cpp_test.cpp']

# Пути к инструментам
config.llvm_tools_dir = "/usr/lib/llvm-21/bin"
config.plugin_path = os.path.join(os.path.dirname(__file__), "..", "stack_check_clang.so")

# Настройка окружения
config.environment['PATH'] = os.path.pathsep.join((
    config.llvm_tools_dir,
    config.environment['PATH']))

# Доступные функции и подстановки
config.available_features.add('clang')
config.substitutions.append(('%shlibdir', os.path.join(os.path.dirname(__file__), "..")))
config.substitutions.append(('%clangxx', os.path.join(config.llvm_tools_dir, 'clang++')))
