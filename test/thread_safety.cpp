// No plugin, no errors
// RUN: %clangxx -I%shlibdir -std=c++20 -fsyntax-only \
// RUN: %s> %p/temp/thread_safety-CPP.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/thread_safety-CPP.cpp.out -check-prefix=CPP --allow-empty
//
// RUN: not %clangxx -I%shlibdir -std=c++20 -fsyntax-only -ferror-limit=500 \
// RUN: -Xclang -load -Xclang %shlibdir/trusted-cpp_clang.so -Xclang -add-plugin -Xclang trust \
// RUN: -Xclang -plugin-arg-trust -Xclang verbose=config \
// RUN: %s > %p/temp/thread_safety-THREAD.cpp.out 2>&1
// RUN: FileCheck %s < %p/temp/thread_safety-THREAD.cpp.out  -check-prefix=THREAD

// CPP-NOT: error

// THREAD: Verbose mode enabled for pattern: 'config'
// THREAD-NOT: warning: declaration does not declare anything [-Wmissing-declarations]
// THREAD-NOT: fatal error
// THREAD-NOT: error: clang frontend command failed
// THREAD-NOT: error: Attribute not processed!

#include "pthread.h"

#include "trusted-cpp.h"
#include <unistd.h>

uint64_t notrust_count = 0;
void *thread_notrust(void *arg) { // Без установки атрибута  THREAD
    ++notrust_count;              // Гонка
    return nullptr;
}

TRUST_THREADSAFE_TYPES("std::atomic");
// TRUST_SET_ATTR_ARGS("threadsafe", "pthread_create", 3, 4);
// TRUST_ATTR_ARGS("threadsafe", "pthread_create", 3, 4);
// TRUST_ATTR_ARGS("threadsafe", "pthread_create");

// TRUST_ATTR(threadsafe, std::atomic);
// TRUST_ATTR(thread, std::thread);
// TRUST_ATTR(thread, std::jthread);

// // Установить атрибут 'threadsafe' для класса trust::SyncTimedShared
// TRUST_ATTR(threadsafe, trust::SyncTimedShared);

// // Установить атрибут 'threadsafe' для всех аргументов конструкторов классов std::thread и std::jthread
// TRUST_ATTR_ARGS(threadsafe, std::thread::thread);
// TRUST_ATTR_ARGS(threadsafe, std::jthread::jthread);

// // Отметить у функции pthread_create атрибутами
// // 'thread' третий аргумент и 'threadsafe' четвертый
// TRUST_ATTR_ARGS(thread, pthread_create, 3);
// TRUST_ATTR_ARGS(threadsafe, pthread_create, 4);

// // Отметить у функции pthread_create атрибутами 'thread' третий аргумент
// TRUST_SET_ATTR_ARGS(thread, pthread_create, 3);

//  TRUST_THREADSAFE
// TRUSTED_PRINT_AST("*");
std::atomic<uint64_t> trust_count = 0;
TRUST_THREAD void *thread_trust(void *arg) {
    trust_count++;
    notrust_count++;
    // THREAD: error: Expected attribute 'threadsafe' for 'notrust_count'
    // THREAD:     notrust_count++;
    // THREAD:     ^
    return nullptr;
}

class TrivialClass {
  public:
    int x;
    int y;
    TrivialClass(const TrivialClass &other) = default;
    TrivialClass(int xx, int yy) : x(xx), y(yy) {}
};
static_assert(std::is_trivially_copyable<TrivialClass>::value);


// LinkageSpecDecl 0x7b28ff7d3620 </home/rsashka/SOURCE/afteri/trusted-cpp/test/thread_safety.cpp:77:1, col:21> col:8 C
// `-VarDecl 0x7b28ff7d 3688 <col:12, col:21> col:21 used ref_counter 'uint64_t':'unsigned long'
//TRUSTED_PRINT_AST("*");
extern "C" uint64_t ref_counter;
TRUST_THREAD TrivialClass thread_trivial(int value, TrivialClass cls, uint64_t *ref) {
    cls.x += ref_counter;
    cls.y += value;
    return cls;
}

void stub() {
    pthread_attr_t attr;
    pthread_attr_init(&attr); // дефолтные значения атрибутов

    pthread_t tid;
    pthread_create(&tid, &attr, thread_notrust, nullptr);
    // THREAD: error: Expected attribute: 'thread' for 3 argument
    // THREAD-NEXT:     pthread_create(&tid, &attr, thread_notrust, nullptr);
    // THREAD-NEXT:                                 ^
    // THREAD-NEXT: verbose: nullptr is 'threadsafe' always.

    pthread_create(&tid, &attr, thread_trust, nullptr);
    // THREAD: verbose: The 'thread' attribute of the 'thread_trust' is present.
    // THREAD-NEXT: verbose: nullptr is 'threadsafe' always.

    {
        std::thread trivial(thread_trivial, 100, TrivialClass(10, 20), &notrust_count);
        trivial.join();
    }

    {
        unsigned long long g_count = 0;
        std::thread t_lambda([&]() {
            // THREAD: verbose: Setting the 'thread' attribute for a lambda function
            for (auto i = 0; i < 1'000'0000; ++i)
                ++g_count;
            // THREAD: error: Expected attribute 'threadsafe' for 'g_count'
            // THREAD:              ++g_count;
            // THREAD:                ^
        });

        std::thread t_notrust(thread_notrust, nullptr);
        // THREAD: error: Expected attribute: 'thread' for 1 argument
        // THREAD-NEXT:        std::thread t_notrust(thread_notrust, nullptr);
        // THREAD-NEXT:                              ^
        // THREAD-NEXT: verbose: nullptr is 'threadsafe' always.

        std::thread t_trust(thread_trust, nullptr);
        // THREAD: verbose: The 'thread' attribute of the 'thread_trust' is present.
        // THREAD-NEXT: verbose: nullptr is 'threadsafe' always.

        t_lambda.join();
        t_notrust.join();
        t_trust.join();
    }

    {
        std::atomic<unsigned long long> a_count = 0;
        std::thread t3([&]() {
            // THREAD: verbose: Setting the 'thread' attribute for a lambda function
            for (auto i = 0; i < 1'000'000; ++i)
                a_count.fetch_add(1);
            // THREAD: verbose: The 'threadsafe' attribute of the 'a_count' is present.
        });

        std::thread t4([&]() {
            // THREAD: verbose: Setting the 'thread' attribute for a lambda function
            for (auto i = 0; i < 1'000'000; ++i)
                a_count++;
            // THREAD: verbose: The 'threadsafe' attribute of the 'a_count' is present.
        });

        t3.join();
        t4.join();
    }

    // {
    //     trust::Shared<int, SyncSingleThread> var_single(0);
    //     bool catched = false;

    //     std::thread other([&]() {
    //         try {
    //             var_single.lock();
    //         } catch (...) {
    //             catched = true;
    //         }
    //     });
    //     other.join();

    //     ASSERT_TRUE(catched);
    // }

    // {
    //     trust::Shared<int, SyncTimedMutex> var_mutex(0);
    //     bool catched = false;

    //     std::chrono::duration<double, std::milli> elapsed;

    //     std::thread other([&]() {
    //         try {

    //             const auto start = std::chrono::high_resolution_clock::now();
    //             std::this_thread::sleep_for(100ms);
    //             elapsed = std::chrono::high_resolution_clock::now() - start;
    //             var_mutex.lock();
    //         } catch (...) {
    //             catched = true;
    //         }
    //     });
    //     ASSERT_TRUE(other.joinable());
    //     other.join();

    //     ASSERT_TRUE(elapsed <= 200.0ms);

    //     ASSERT_FALSE(catched);
    // }

    // {
    //     trust::Shared<int, SyncTimedShared> var_recursive(0);
    //     bool not_catched = true;

    //     std::thread read([&]() {
    //         try {
    //             auto a1 = var_recursive.lock_const();
    //             auto a2 = var_recursive.lock_const();
    //             auto a3 = var_recursive.lock_const();
    //         } catch (...) {
    //             not_catched = false;
    //         }
    //     });
    //     read.join();

    //     ASSERT_TRUE(not_catched);

    //     std::thread write([&]() {
    //         try {
    //             auto a1 = var_recursive.lock();
    //             auto a2 = var_recursive.lock();
    //             auto a3 = var_recursive.lock();
    //         } catch (...) {
    //             not_catched = false;
    //         }
    //     });
    //     write.join();

    //     ASSERT_FALSE(not_catched);
    // }
}

// THREAD: verbose(config): plugin-config
//  ...
// THREAD: verbose(config): thread: 'thread_trust'
// THREAD-NEXT: verbose(config): threadsafe: 'std::atomic'
// THREAD-NEXT: verbose(config): attr-args: pthread_create(thread=3,threadsafe=4), std::thread::jthread(thread=1,threadsafe=...),
// std::thread::thread(thread=1,threadsafe=...)
