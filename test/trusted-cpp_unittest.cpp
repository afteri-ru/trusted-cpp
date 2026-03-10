#include <cstdio>
#include <vector>
#ifndef BUILD_UNITTEST
#error "Build for unt test only"
#endif

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "pthread.h"

#include "trusted-cpp_plugin.h"

using namespace trust;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

TEST(Trust, Sizes) {

    EXPECT_EQ(32, sizeof(std::string));
    EXPECT_EQ(32, sizeof(std::wstring));
    EXPECT_EQ(40, sizeof(std::variant<std::string>));
    EXPECT_EQ(40, sizeof(std::variant<std::string, std::wstring>));

    EXPECT_EQ(16, sizeof(std::runtime_error));
    EXPECT_EQ(16, sizeof(trust_error));

    class TestClassV0 {

        void func() {}
    };

    class TestClassV1 {

        virtual void func() {}
    };

    class TestClassV2 {

        virtual void func1() {}

        virtual void func2() {}
    };

    class TestClassV3 {

        TestClassV3() {}

        virtual void func1() {}

        virtual void func2() {}

        virtual TestClassV3 &operator=(TestClassV3 &) = 0;

        virtual ~TestClassV3() {}
    };

    class TestClass1 : std::shared_ptr<int> {};

    class TestClass2 : std::shared_ptr<int>, std::enable_shared_from_this<TestClass2> {};

    class TestClass3 : std::enable_shared_from_this<TestClass3> {
        int value;
    };

    class TestClass4 : std::enable_shared_from_this<TestClass4> {};

    EXPECT_EQ(1, sizeof(TestClassV0));
    EXPECT_EQ(8, sizeof(TestClassV1));
    EXPECT_EQ(8, sizeof(TestClassV2));
    EXPECT_EQ(8, sizeof(TestClassV3));

    EXPECT_EQ(16, sizeof(TestClass1));
    EXPECT_EQ(32, sizeof(TestClass2));
    EXPECT_EQ(4, sizeof(int));
    EXPECT_EQ(24, sizeof(TestClass3));
    EXPECT_EQ(16, sizeof(TestClass4));

    EXPECT_EQ(16, sizeof(Sync<int>));
    EXPECT_EQ(24, sizeof(SyncSingleThread<int>));
    EXPECT_EQ(56, sizeof(SyncTimedMutex<int>));
    EXPECT_EQ(72, sizeof(SyncTimedShared<int>));

    EXPECT_EQ(16, sizeof(Sync<uint8_t>));
    EXPECT_EQ(24, sizeof(SyncSingleThread<uint8_t>));
    EXPECT_EQ(56, sizeof(SyncTimedMutex<uint8_t>));
    EXPECT_EQ(72, sizeof(SyncTimedShared<uint8_t>));

    EXPECT_EQ(24, sizeof(Sync<uint64_t>));
    EXPECT_EQ(32, sizeof(SyncSingleThread<uint64_t>));
    EXPECT_EQ(64, sizeof(SyncTimedMutex<uint64_t>));
    EXPECT_EQ(80, sizeof(SyncTimedShared<uint64_t>));

    EXPECT_EQ(99, sizeof(std::array<uint8_t, 99>));
    EXPECT_EQ(100, sizeof(std::array<uint8_t, 100>));
    EXPECT_EQ(101, sizeof(std::array<uint8_t, 101>));

    EXPECT_EQ(112, sizeof(Sync<std::array<uint8_t, 100>>));
    EXPECT_EQ(120, sizeof(SyncSingleThread<std::array<uint8_t, 100>>));
    EXPECT_EQ(152, sizeof(SyncTimedMutex<std::array<uint8_t, 100>>));
    EXPECT_EQ(168, sizeof(SyncTimedShared<std::array<uint8_t, 100>>));

    EXPECT_EQ(101, sizeof(std::array<uint8_t, 101>));
    EXPECT_EQ(112, sizeof(Sync<std::array<uint8_t, 101>>));
    EXPECT_EQ(120, sizeof(SyncSingleThread<std::array<uint8_t, 101>>));
    EXPECT_EQ(152, sizeof(SyncTimedMutex<std::array<uint8_t, 101>>));
    EXPECT_EQ(168, sizeof(SyncTimedShared<std::array<uint8_t, 101>>));

    EXPECT_EQ(16, sizeof(Shared<int>));
    EXPECT_EQ(16, sizeof(Shared<int, SyncSingleThread>));
    EXPECT_EQ(16, sizeof(Shared<int, SyncTimedMutex>));
    EXPECT_EQ(16, sizeof(Shared<int, SyncTimedShared>));

    EXPECT_EQ(16, sizeof(Shared<uint8_t>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncSingleThread>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncTimedMutex>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncTimedShared>));

    EXPECT_EQ(16, sizeof(Shared<uint8_t>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncSingleThread>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncTimedMutex>));
    EXPECT_EQ(16, sizeof(Shared<uint8_t, SyncTimedShared>));

    EXPECT_EQ(16, sizeof(Weak<Shared<int>>));
    EXPECT_EQ(16, sizeof(Weak<Shared<int, SyncSingleThread>>));
    EXPECT_EQ(16, sizeof(Weak<Shared<int, SyncTimedMutex>>));
    EXPECT_EQ(16, sizeof(Weak<Shared<int, SyncTimedShared>>));

    EXPECT_EQ(16, sizeof(Locker<int, Shared<int>>));
    EXPECT_EQ(16, sizeof(Locker<int, Shared<int, SyncSingleThread>>));
    EXPECT_EQ(16, sizeof(Locker<int, Shared<int, SyncTimedMutex>>));
    EXPECT_EQ(16, sizeof(Locker<int, Shared<int, SyncTimedShared>>));

    EXPECT_EQ(8, sizeof(Locker<int, int &>));
    EXPECT_EQ(8, sizeof(Locker<uint8_t, uint8_t &>));
    EXPECT_EQ(8, sizeof(Locker<uint16_t, uint16_t &>));
    EXPECT_EQ(8, sizeof(Locker<uint32_t, uint32_t &>));
    EXPECT_EQ(8, sizeof(Locker<uint64_t, uint64_t &>));
}

TEST(TrustedCPP, Cast) {

    EXPECT_EQ(1, sizeof(bool));
    EXPECT_EQ(1, sizeof(Value<bool>));
    EXPECT_EQ(4, sizeof(int32_t));
    EXPECT_EQ(4, sizeof(Value<int32_t>));
    EXPECT_EQ(8, sizeof(int64_t));
    EXPECT_EQ(8, sizeof(Value<int64_t>));

    Value<int> value_int(0);
    int &take_value = *value_int;
    Locker<int, int &> take_value2 = value_int.lock();

    Shared<int> shared_int(0);

    //    Auto<int, Shared<int>> take_shared = *shared_int;
    Locker<int, Shared<int>::SharedType> take_shared1 = shared_int.lock();
    //    int & take_shared1 = *shared_int;

    ASSERT_EQ(0, *take_shared1);
    ASSERT_EQ(0, *shared_int.lock());
    //    ASSERT_EQ(0, **shared_int);
    *shared_int.lock() = 11;
    //    **shared_int = 11;
    //    ASSERT_EQ(11, **shared_int);
    ASSERT_EQ(11, *shared_int.lock());

    auto var_take_shared = shared_int.lock();
    int &take_shared2 = *var_take_shared;

    Shared<int, SyncSingleThread> sync_int(22);
    Locker<int, Shared<int, SyncSingleThread>::SharedType> take_sync_int = sync_int.lock();
    auto auto_sync_int = sync_int.lock();
    //    auto auto_sync_int = *sync_int;

    ASSERT_EQ(22, *sync_int.lock());
    //    ASSERT_EQ(22, **sync_int);

    *auto_sync_int = 33;
    ASSERT_EQ(33, *auto_sync_int);
    ASSERT_EQ(33, *sync_int.lock());
    //    ASSERT_EQ(33, **sync_int);

    int temp_sync = *sync_int.lock();
    *sync_int.lock() = 44;
    //    **sync_int = 44;

    ASSERT_EQ(33, temp_sync);
    ASSERT_EQ(44, *auto_sync_int);
    //    ASSERT_EQ(44, **sync_int);
    ASSERT_EQ(44, *sync_int.lock());

    auto weak_shared(shared_int.weak());
    Weak<Shared<int>> weak_shared1(shared_int.weak());
    Weak<Shared<int>> weak_shared2 = weak_shared1;
    auto weak_shared3 = shared_int.weak();

    Weak<Shared<int>> weak_shared4 = shared_int.weak();

    ASSERT_EQ(3, sync_int.use_count()) << sync_int.use_count();
    ASSERT_NO_THROW(sync_int.weak());
    Weak<Shared<int, SyncSingleThread>> weak_sync1 = sync_int.weak();
    //
    //    //    std::cout << "weak_guard1: " << guard_int.shared_this.use_count() << "\n";
    //
    auto weak_sync2(sync_int.weak());
    //
    //    //    std::cout << "weak_guard2: " << guard_int.shared_this.use_count() << "\n";
    //
    auto weak_sync3 = sync_int.weak();

    //    std::cout << "weak_guard3: " << guard_int.shared_this.use_count() << "\n";

    // template <typename V, typename T = V, typename W = std::weak_ptr<T>> class VarWeak;
    //     ASSERT_NO_THROW(**weak_shared1);
    ASSERT_NO_THROW(*weak_shared1.lock());

    Locker<int, Shared<int>::SharedType> auto_shared(weak_shared1.lock());
    ASSERT_EQ(3, sync_int.use_count());

    //    auto auto_guard1(weak_guard1.take());

    //    ASSERT_NO_THROW(**sync_int);
    ASSERT_NO_THROW(*sync_int.lock());
    {
        auto taken = sync_int.lock();
    }

    //    VarAuto<int, VarGuard<int, VarGuardData<int>>> auto_guard2(weak_guard1.take());
    //    std::cout << "auto_guard: " << guard_int.shared_this.use_count() << "\n";

    //    int & take_weak_shared = *auto_shared;
    //    int & take_weak_guard = *auto_guard;
}

#pragma clang attribute push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

class TestClass1 {
  public:
    Class<TestClass1> field;
    Class<TestClass1> field_2;

    Shared<TestClass1> m_shared;
    Shared<TestClass1, SyncSingleThread> m_single;
    Shared<TestClass1, SyncTimedMutex> m_mutex;
    Shared<TestClass1, SyncTimedShared> m_recursive;

    TestClass1() : field(*this, this->field), field_2(*this, this->field_2) {}
};

#pragma clang attribute pop

TEST(TrustedCPP, Class) {

    TestClass1 cls;

    // Ignore warning only for unit test from field Shared<TestClass1>
#pragma clang attribute push
#pragma clang diagnostic ignored "-Winvalid-offsetof"

    ASSERT_EQ(0, offsetof(TestClass1, field));
    ASSERT_NE(0, offsetof(TestClass1, field_2));

    ASSERT_EQ((size_t)cls.field.m_instance, (size_t)&cls);
    ASSERT_EQ((size_t)cls.field.m_offset, offsetof(TestClass1, field)) << offsetof(TestClass1, field);

    ASSERT_EQ((size_t)cls.field_2.m_instance, (size_t)&cls);
    ASSERT_EQ(cls.field_2.m_offset, offsetof(TestClass1, field_2)) << offsetof(TestClass1, field_2);

#pragma clang attribute pop

    ASSERT_FALSE(cls.field.m_field.get());
    cls.field = new TestClass1();
    ASSERT_TRUE(cls.field.m_field.get());

    ASSERT_FALSE((*cls.field).field.m_field.get());
    ASSERT_ANY_THROW((*cls.field).field = cls.field; // circular reference
    );
    ASSERT_TRUE(cls.field.m_field.get());

    ASSERT_FALSE((*cls.field).field.m_field.get());
    ASSERT_ANY_THROW((*cls.field).field = cls.field.m_field.get(); // circular reference
    );
    ASSERT_TRUE(cls.field.m_field.get());

    ASSERT_FALSE(cls.field_2.m_field.get());
    ASSERT_ANY_THROW(cls.field_2 = cls.field; // Copy of another field
    );
    ASSERT_FALSE(cls.field_2.m_field.get());
};

unsigned long long g_count = 0;
void * no_safe_thread() {
    for (auto i = 0; i < 1'000'0000; ++i) {
        ++g_count;
    }
    return nullptr;
}
TEST(TrustedCPP, Threads) {

    {
        std::thread t1([&]() {
            for (auto i = 0; i < 1'000'0000; ++i)
                ++g_count;
        });
        std::thread t2([&]() {
            for (auto i = 0; i < 1'000'0000; ++i)
                ++g_count;
        });
        std::thread t3(&no_safe_thread);
        t1.join();
        t2.join();
        t3.join();

        EXPECT_NE(3'000'000, g_count);
    }

    {
        std::atomic<unsigned long long> a_count{0};
        std::thread t3([&]() {
            for (auto i = 0; i < 1'000'000; ++i)
                a_count.fetch_add(1);
        });

        std::thread t4([&]() {
            for (auto i = 0; i < 1'000'000; ++i)
                a_count.fetch_add(1);
        });

        t3.join();
        t4.join();

        EXPECT_EQ(2'000'000, a_count);
    }

    {
        trust::Shared<int, SyncSingleThread> var_single(0);
        bool catched = false;

        std::thread other([&]() {
            try {
                var_single.lock();
            } catch (...) {
                catched = true;
            }
        });
        other.join();

        ASSERT_TRUE(catched);
    }

    {
        trust::Shared<int, SyncTimedMutex> var_mutex(0);
        bool catched = false;

        std::chrono::duration<double, std::milli> elapsed;

        std::thread other([&]() {
            try {

                const auto start = std::chrono::high_resolution_clock::now();
                std::this_thread::sleep_for(100ms);
                elapsed = std::chrono::high_resolution_clock::now() - start;
                var_mutex.lock();
            } catch (...) {
                catched = true;
            }
        });
        ASSERT_TRUE(other.joinable());
        other.join();

        ASSERT_TRUE(elapsed <= 200.0ms);

        ASSERT_FALSE(catched);
    }

    {
        trust::Shared<int, SyncTimedShared> var_recursive(0);
        bool not_catched = true;

        std::thread read([&]() {
            try {
                auto a1 = var_recursive.lock_const();
                auto a2 = var_recursive.lock_const();
                auto a3 = var_recursive.lock_const();
            } catch (...) {
                not_catched = false;
            }
        });
        read.join();

        ASSERT_TRUE(not_catched);

        std::thread write([&]() {
            try {
                auto a1 = var_recursive.lock();
                auto a2 = var_recursive.lock();
                auto a3 = var_recursive.lock();
            } catch (...) {
                not_catched = false;
            }
        });
        write.join();

        ASSERT_FALSE(not_catched);
    }
}

TEST(TrustedCPP, Depend) {
    {
        std::vector<int> vect(100000, 0);
        auto b = vect.begin();
        auto e = vect.end();

        EXPECT_EQ(8, sizeof(b));
        EXPECT_EQ(8, sizeof(e));

        vect = {};
        //        vect.shrink_to_fit();
        std::sort(b, e);
    }
    {
        std::vector<int> vect(100000, 0);

        auto b = LazyCaller<decltype(vect), decltype(std::declval<decltype(vect)>().begin())>(vect, &decltype(vect)::begin);
        auto e = LAZYCALL(vect, end);
        auto c = LAZYCALL(vect, clear);
        auto s = LAZYCALL(vect, size);

        EXPECT_EQ(32, sizeof(b));
        EXPECT_EQ(32, sizeof(e));

        ASSERT_EQ(100000, vect.size());
        ASSERT_EQ(100000, *s);

        *c;

        ASSERT_EQ(0, vect.size());
        ASSERT_EQ(0, *s);

        // std::vector<int> data(1, 1);
        // auto reserve = LAZYCALL(vect, reserve, 10UL);
        // *reserve;
        //
        // auto db = LAZYCALL(data, begin);
        // auto de = LAZYCALL(data, end);
        //
        // ASSERT_EQ(1, data[0]);
        // // @todo variadic tempalte as variadic template argument
        // auto call = LAZYCALL(vect, assign, db, de);

        // data.clear();
        // data.shrink_to_fit();
        // *call;

        // ASSERT_EQ(1, vect.size());
        // ASSERT_EQ(1, *s);
        //
        // ASSERT_EQ(1, data[0]);
        // ASSERT_EQ(1, vect[0]);

        vect.shrink_to_fit();
        std::sort(*b, *e);
    }
}

TEST(TrustedCPP, ApplyAttr) {

    trust::Value<int> var_value(1);
    static trust::Value<int> var_static(1);

    var_static = var_value;
    {
        var_static = var_value;
        {
            var_static = var_value;
        }
    }

    trust::Shared<int> var_shared1(0);
    trust::Shared<int> var_shared2(1);

    ASSERT_TRUE(var_shared1);
    ASSERT_TRUE(var_shared2);

    var_shared1 = var_shared2;
    {
        var_shared1 = var_shared2;
        {
            var_shared1 = var_shared2;
        }
    }

    trust::Shared<int, SyncSingleThread> var_none(1);
    ASSERT_TRUE(var_none);

    trust::Shared<int, SyncTimedMutex> var_mutex(1);
    ASSERT_TRUE(var_mutex);

    trust::Shared<int, SyncTimedShared> var_shared(1);
    ASSERT_TRUE(var_shared);
}

TEST(TrustedCPP, Separartor) {
    ASSERT_STREQ("", SeparatorRemove("").c_str());
    ASSERT_STREQ("0", SeparatorRemove("0").c_str());
    ASSERT_STREQ("00", SeparatorRemove("0'0").c_str());
    ASSERT_STREQ("0000", SeparatorRemove("0000").c_str());
    ASSERT_STREQ("000000", SeparatorRemove("0_00_000").c_str());
    ASSERT_STREQ("00000000", SeparatorRemove("0'0'0'0'0'0'0'0").c_str());

    EXPECT_STREQ("0", SeparatorInsert(0).c_str());
    EXPECT_STREQ("1", SeparatorInsert(1).c_str());
    EXPECT_STREQ("111", SeparatorInsert(111).c_str());
    EXPECT_STREQ("1'111", SeparatorInsert(1'111).c_str());
    EXPECT_STREQ("11'111", SeparatorInsert(11'111).c_str());
    EXPECT_STREQ("111'111", SeparatorInsert(111'111).c_str());
    EXPECT_STREQ("111_111_111_111", SeparatorInsert(111'111'111'111, '_').c_str());
}

TEST(TrustedCPP, TrustFile) {

    std::string filename = "unittest-circleref.trust";

    fs::remove(filename);
    ASSERT_FALSE(fs::exists(filename));

    TrustFile::ClassReadType classes;

    {
        TrustFile file(filename, "file_empty.cpp");

        file.WriteFile(classes);

        ASSERT_TRUE(fs::exists(filename));
        TrustFile::ClassReadType readed;
        ASSERT_NO_THROW(file.ReadFile(readed));
        ASSERT_TRUE(readed.empty());
    }
    {
        TrustFile file(filename, "file1.cpp");

        classes["class0"] = {};

        classes["class1"].parents = {
            {"ns::class1", "filepos:1"},
            {"ns::class2", "filepos:2"},
        };
        classes["class1"].fields = {
            {"ns::field1", "filepos:1"},
            {"ns::field2", "filepos:2"},
        };

        file.WriteFile(classes);

        ASSERT_TRUE(fs::exists(filename));

        {
            TrustFile file(filename, "file_read.cpp");

            TrustFile::ClassReadType readed;

            ASSERT_NO_THROW(file.ReadFile(readed));

            ASSERT_EQ(2, readed.size());
            ASSERT_EQ(0, readed["class0"].parents.size());
            ASSERT_EQ(0, readed["class0"].fields.size());
            ASSERT_EQ(2, readed["class1"].parents.size());
            ASSERT_EQ(2, readed["class1"].fields.size());

            ASSERT_STREQ("filepos:1", readed["class1"].parents["ns::class1"].c_str());
            ASSERT_STREQ("filepos:2", readed["class1"].parents["ns::class2"].c_str());
            ASSERT_STREQ("filepos:1", readed["class1"].fields["ns::field1"].c_str());
            ASSERT_STREQ("filepos:2", readed["class1"].fields["ns::field2"].c_str());
        }
    }
    {
        TrustFile file(filename, "file2.cpp");

        ASSERT_TRUE(fs::exists(filename));

        classes.erase(classes.find("class0"));

        classes["class1"].parents = {
            {"ns::class1", "filepos:111"},
            {"ns::class2", "filepos:222"},
        };
        classes["class1"].fields = {
            {"ns::field1", "filepos:111"},
            {"ns::field2", "filepos:222"},
        };

        file.WriteFile(classes);

        ASSERT_TRUE(fs::exists(filename));

        TrustFile::ClassReadType readed;

        ASSERT_NO_THROW(file.ReadFile(readed));

        ASSERT_EQ(2, readed.size());
        ASSERT_EQ(0, readed["class0"].parents.size());
        ASSERT_EQ(0, readed["class0"].fields.size());
        ASSERT_EQ(2, readed["class1"].parents.size());
        ASSERT_EQ(2, readed["class1"].fields.size());

        ASSERT_STREQ("filepos:1", readed["class1"].parents["ns::class1"].c_str());
        ASSERT_STREQ("filepos:2", readed["class1"].parents["ns::class2"].c_str());
        ASSERT_STREQ("filepos:1", readed["class1"].fields["ns::field1"].c_str());
        ASSERT_STREQ("filepos:2", readed["class1"].fields["ns::field2"].c_str());
    }
    fs::remove(filename);
    filename += ".bak";
    fs::remove(filename);
}

TEST(TrustedCPP, WeakList) {

    LinkedWeakList<int> int_list;

    ASSERT_TRUE(int_list.empty());
    ASSERT_STREQ("nullptr", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(0, int_list.size());
    int_list.push_back(1);
    ASSERT_STREQ("1 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(1, int_list.size());
    int_list.push_back(2);
    ASSERT_STREQ("1 -> 2 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(2, int_list.size());
    int_list.push_back(3);
    ASSERT_STREQ("1 -> 2 -> 3 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(3, int_list.size());
    int_list.push_front(0);
    int_list.push_front(0);
    ASSERT_STREQ("0 -> 0 -> 1 -> 2 -> 3 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(5, int_list.size());
    int_list.pop_front();
    ASSERT_STREQ("0 -> 1 -> 2 -> 3 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(4, int_list.size());
    int_list.pop_back();
    ASSERT_STREQ("0 -> 1 -> 2 -> ", int_list.to_string().c_str()) << int_list.to_string();
    ASSERT_EQ(3, int_list.size());
    ASSERT_FALSE(int_list.empty());
}

TEST(StringMatcher, Patterns) {

    StringMatcher empty;
    EXPECT_TRUE(empty.isEmpty());

    StringMatcher match_0("");

    EXPECT_FALSE(match_0.isEmpty());
    EXPECT_TRUE(match_0.MatchesName(""));
    EXPECT_FALSE(match_0.MatchesName("_"));
    EXPECT_FALSE(match_0.MatchesName("name"));
    EXPECT_FALSE(match_0.MatchesName("name::"));
    EXPECT_FALSE(match_0.MatchesName("::name"));
    EXPECT_FALSE(match_0.MatchesName("ns::name"));

    StringMatcher match_1("*");

    EXPECT_FALSE(match_1.isEmpty());
    EXPECT_TRUE(match_1.MatchesName(""));
    EXPECT_TRUE(match_1.MatchesName("_"));
    EXPECT_TRUE(match_1.MatchesName("name"));
    EXPECT_TRUE(match_1.MatchesName("name::"));
    EXPECT_TRUE(match_1.MatchesName("::name"));
    EXPECT_TRUE(match_1.MatchesName("ns::name"));

    StringMatcher match_none("_");

    EXPECT_FALSE(match_none.isEmpty());
    EXPECT_FALSE(match_none.MatchesName(""));
    EXPECT_TRUE(match_none.MatchesName("_"));
    EXPECT_FALSE(match_none.MatchesName("name"));
    EXPECT_FALSE(match_none.MatchesName("name::"));
    EXPECT_FALSE(match_none.MatchesName("::name"));
    EXPECT_FALSE(match_none.MatchesName("ns::name"));

    StringMatcher match_ns("*global_*");

    EXPECT_FALSE(match_ns.isEmpty());
    EXPECT_FALSE(match_ns.MatchesName(""));
    EXPECT_FALSE(match_ns.MatchesName("global"));
    EXPECT_TRUE(match_ns.MatchesName("ns::global_1"));
    EXPECT_TRUE(match_ns.MatchesName("ns::ns2::global_2"));

    StringMatcher match_ns2("ns*");

    EXPECT_FALSE(match_ns2.isEmpty());
    EXPECT_FALSE(match_ns2.MatchesName(""));
    EXPECT_FALSE(match_ns2.MatchesName("global"));
    EXPECT_TRUE(match_ns2.MatchesName("ns::global_1"));
    EXPECT_TRUE(match_ns2.MatchesName("ns::ns2::global_2"));

    StringMatcher match_name("ns::global_1;*global_2");

    EXPECT_FALSE(match_name.isEmpty());
    EXPECT_FALSE(match_name.MatchesName(""));
    EXPECT_FALSE(match_name.MatchesName("global"));
    EXPECT_TRUE(match_name.MatchesName("ns::global_1"));
    EXPECT_TRUE(match_name.MatchesName("ns::ns2::global_2"));

    StringMatcher match_space("ns::global_1; *global_2");

    EXPECT_FALSE(match_space.isEmpty());
    EXPECT_FALSE(match_space.MatchesName(""));
    EXPECT_FALSE(match_space.MatchesName("global"));
    EXPECT_TRUE(match_space.MatchesName("ns::global_1"));
    // EXPECT_TRUE(match_space.MatchesName("ns::ns2::global_2"));
}

uint64_t notrust_count = 0;
[[clang::optnone]] void *thread_notrust(void *arg) { // Без установки атрибута  THREAD

    for (int64_t i = 0; i < 1'000'0000; i++) {
        ++notrust_count; // Гонка
    }
    return nullptr;
}

int64_t run_therad_not_safe(int count_thread) {
    pthread_attr_t attr;
    pthread_attr_init(&attr); // дефолтные значения атрибутов

    std::vector<pthread_t> threads(count_thread);

    for (pthread_t &tid : threads) {
        pthread_create(&tid, &attr, thread_notrust, nullptr); // OK
    }

    for (pthread_t &tid : threads) {
        pthread_join(tid, nullptr);
    }

    return notrust_count;
}

TEST(PThread, NOTRUST) {

    ASSERT_EQ(1'000'0000, run_therad_not_safe(1));
    ASSERT_NE(3'000'0000, run_therad_not_safe(2));
    ASSERT_NE(6'000'0000, run_therad_not_safe(3));
}

std::atomic<uint64_t> trust_count = 0;
[[clang::optnone]] void *thread_trust(void *arg) { // Без установки атрибута  THREAD

    for (int64_t i = 0; i < 1'000'0000; i++) {
        trust_count++;
    }
    return nullptr;
}

int64_t run_therad_safe(int count_thread) {
    pthread_attr_t attr;
    pthread_attr_init(&attr); // дефолтные значения атрибутов

    std::vector<pthread_t> threads(count_thread);

    for (pthread_t &tid : threads) {
        pthread_create(&tid, &attr, thread_trust, nullptr); // OK
    }

    for (pthread_t &tid : threads) {
        pthread_join(tid, nullptr);
    }

    return trust_count;
}

TEST(PThread, TRUST) {

    ASSERT_EQ(1'000'0000, run_therad_safe(1));
    ASSERT_EQ(3'000'0000, run_therad_safe(2));
    ASSERT_EQ(6'000'0000, run_therad_safe(3));
}

// THREADSAFE std::atomic<unsigned long long> a_count{0};
// THREAD void *thread_func(void *arg) {
//     TRUST_USING_EXTERNAL("*") // Разрешить доступ к любым внешним переменным
//     a_count.fetch_add(1);     // OK
//     pthread_exit(0);
// }

// unsigned long long run_therad_safe() {

//     pthread_t tid;
//     pthread_attr_t attr;
//     // дефолтные значения атрибутов
//     pthread_attr_init(&attr);

//     // Ошибка компиляции pthread_create
//     // Третий аргумент должен иметь атрубут 'thread'
//     // pthread_create(&tid, &attr, thread_func_notrust, nullptr);

//     pthread_create(&tid, &attr, thread_func, nullptr); // OK

//     pthread_join(tid, nullptr);

//     return a_count;
// }
