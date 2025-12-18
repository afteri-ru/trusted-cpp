---
layout: default
title: Примеры
lang: ru
---

# Примеры использования Trusted-CPP

Здесь представлены некоторые примеры, демонстрирующие, как использовать Trusted-CPP в ваших проектах.

## Базовый пример использования

Для компиляции файла с плагином Trusted-CPP используйте следующую командную строку:

```bash
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-disable _example.cpp
```

## Обнаружение инвалидации указателей

Плагин может обнаруживать инвалидацию ссылочных переменных после изменения данных в основной переменной:

```cpp
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> vec(100000, 0);
    auto x = vec.begin();
    vec = {};                // <-- Плагин предупредит об использовании основной переменной 'vec'
    vec.shrink_to_fit();     // <-- Плагин предупредит об использовании основной переменной 'vec'
    std::sort(x, vec.end()); // <-- Плагин сообщит об ошибке: Using dependent variable 'x' after changing main variable 'vec'!
    return 0;
}
```

Вывод плагина:
```
_example.cpp:4:17: warning: using main variable 'vec'
    vec = {};
                ^
_example.cpp:5:17: warning: using main variable 'vec'
    vec.shrink_to_fit();
                ^
_example.cpp:6:27: error: Using the dependent variable 'x' after changing the main variable 'vec'!
    std::sort(x, vec.end());
                          ^
```

## Обнаружение циклических ссылок

Плагин также может обнаруживать циклические ссылки между классами:

```cpp
class SharedCross2;  // Предварительное объявление

class SharedCross {
    SharedCross2 *cross2;  // <-- Плагин обнаружит это как часть циклической ссылки
};

class SharedCross2 {
    SharedCross *cross;    // <-- Плагин обнаружит это как часть циклической ссылки
};
```

Вывод плагина:
```
_cycles.cpp:53:23: error: The class 'cycles::SharedCross' has a circular reference through class 'cycles::SharedCross2'
    SharedCross2 *cross2;
                      ^
_cycles.cpp:57:22: error: The class 'cycles::SharedCross2' has a circular reference through class 'cycles::SharedCross'
    SharedCross *cross;
                     ^
```

## Пример использования библиотеки

Использование библиотеки в вашем коде в виде заголовочных файлов:

```cpp
#include "memsafe.h"

// Определение общей переменной с потоковой безопасностью
threadsafe_shared_var<std::vector<int>> myVector;

void worker_thread() {
    // Безопасный доступ к общей переменной
    auto safe_access = lock(myVector);
    safe_access->push_back(42);
    // Блокировка автоматически снимается в конце области видимости
}
```

## Параметры компиляции

Плагин поддерживает несколько аргументов командной строки:

- `--circleref-disable`: Отключить анализ циклических ссылок
- `--circleref-write`: Записать информацию о классах для анализа циклических ссылок (первый проход)
- `--circleref-read`: Прочитать информацию о классах для анализа циклических ссылок (второй проход)

Пример с отключенным анализом циклических ссылок:
```bash
clang++ -std=c++20 -Xclang -load -Xclang ./memsafe_clang.so -Xclang -add-plugin -Xclang memsafe -Xclang -plugin-arg-memsafe -Xclang circleref-disable _example.cpp
```
