#include "w6_mem/allocator.h"
#include "w6_mem/unique_ptr.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>

namespace w6_mem {
namespace {

// テスト用のクラス
class TestClass {
public:
    TestClass() : m_value(0) {}
    explicit TestClass(int value) : m_value(value) {}
    int get_value() const {
        return m_value;
    }
    void set_value(int value) {
        m_value = value;
    }

private:
    int m_value;
};

// デストラクタ呼び出しを追跡するためのクラス
class DestructorTracker {
public:
    static inline int m_destructor_calls = 0;

    DestructorTracker() = default;
    DestructorTracker(const DestructorTracker&) = delete;
    DestructorTracker& operator=(const DestructorTracker&) = delete;
    ~DestructorTracker() {
        ++m_destructor_calls;
    }

    static void reset_counter() {
        m_destructor_calls = 0;
    }
    static int get_counter() {
        return m_destructor_calls;
    }
};

// 継承テスト用の第一基底クラス
class BaseClass {
public:
    static inline int m_destructor_calls = 0;

    BaseClass() : m_value1(0) {}
    explicit BaseClass(int value) : m_value1(value) {}
    BaseClass(const BaseClass&) = delete;
    BaseClass& operator=(const BaseClass&) = delete;
    virtual ~BaseClass() {
        ++m_destructor_calls;
    }

    virtual int get_value1() const {
        return m_value1;
    }

    static void reset_counter() {
        m_destructor_calls = 0;
    }
    static int get_counter() {
        return m_destructor_calls;
    }

private:
    int m_value1;
};

// 継承テスト用の第二基底クラス
class SecondBase {
public:
    static inline int m_destructor_calls = 0;

    SecondBase() : m_value2(0) {}
    explicit SecondBase(int value) : m_value2(value) {}
    SecondBase(const SecondBase&) = delete;
    SecondBase& operator=(const SecondBase&) = delete;
    virtual ~SecondBase() {
        ++m_destructor_calls;
    }

    virtual int get_value2() const {
        return m_value2;
    }

    static void reset_counter() {
        m_destructor_calls = 0;
    }
    static int get_counter() {
        return m_destructor_calls;
    }

private:
    int m_value2;
};

// 継承テスト用の派生クラス（多重継承）
class DerivedClass : public BaseClass, public SecondBase {
public:
    DerivedClass(int value1, int value2, int value3)
        : BaseClass(value1), SecondBase(value2), m_value3(value3) {}

    int get_value1() const override {
        return BaseClass::get_value1();
    }
    int get_value2() const override {
        return SecondBase::get_value2();
    }
    int get_value3() const {
        return m_value3;
    }

private:
    int m_value3;
};

TEST(UniquePtrTest, DefaultConstructor) {
    const UniquePtr<TestClass> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(UniquePtrTest, NullptrConstructor) {
    const UniquePtr<TestClass> ptr(nullptr);
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(UniquePtrTest, MakeUnique) {
    DefaultAllocator allocator;
    auto ptr = make_unique<TestClass>(&allocator, 42);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->get_value(), 42);
}

TEST(UniquePtrTest, MoveConstructor) {
    DefaultAllocator allocator;
    auto ptr1 = make_unique<TestClass>(&allocator, 42);
    const UniquePtr<TestClass> ptr2(std::move(ptr1));

    EXPECT_FALSE(ptr1); // NOLINT
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->get_value(), 42);
}

TEST(UniquePtrTest, MoveAssignment) {
    DefaultAllocator allocator;
    auto ptr1 = make_unique<TestClass>(&allocator, 42);
    auto ptr2 = make_unique<TestClass>(&allocator, 24);

    ptr2 = std::move(ptr1);

    EXPECT_FALSE(ptr1); // NOLINT
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->get_value(), 42);
}

TEST(UniquePtrTest, DestructorCall) {
    DefaultAllocator allocator;
    DestructorTracker::reset_counter();
    {
        auto ptr = make_unique<DestructorTracker>(&allocator);
        EXPECT_EQ(DestructorTracker::get_counter(), 0);
    }
    EXPECT_EQ(DestructorTracker::get_counter(), 1);
}

TEST(UniquePtrTest, OperatorDereference) {
    DefaultAllocator allocator;
    auto ptr = make_unique<TestClass>(&allocator, 42);
    EXPECT_EQ((*ptr).get_value(), 42);
    ptr->set_value(24);
    EXPECT_EQ(ptr->get_value(), 24);
}

TEST(UniquePtrTest, BooleanConversion) {
    DefaultAllocator allocator;
    const UniquePtr<TestClass> null_ptr;
    auto valid_ptr = make_unique<TestClass>(&allocator);

    EXPECT_FALSE(null_ptr);
    EXPECT_TRUE(valid_ptr);
}

TEST(UniquePtrTest, CustomAllocatorMemoryManagement) {
    DefaultAllocator allocator;
    DestructorTracker::reset_counter();
    {
        auto ptr = make_unique<DestructorTracker>(&allocator);
        EXPECT_EQ(DestructorTracker::get_counter(), 0);
    }
    EXPECT_EQ(DestructorTracker::get_counter(), 1);
}

TEST(UniquePtrTest, MultipleInheritanceToFirstBase) {
    DefaultAllocator allocator;
    BaseClass::reset_counter();
    {
        // 派生クラスのポインタを第一基底クラスのUniquePtrに移動
        UniquePtr<BaseClass> base_ptr = make_unique<DerivedClass>(&allocator, 42, 24, 10);
        EXPECT_EQ(BaseClass::get_counter(), 0);
        EXPECT_EQ(base_ptr->get_value1(), 42);
    }
    // スコープを抜けたときに仮想デストラクタが正しく呼ばれることを確認
    EXPECT_EQ(BaseClass::get_counter(), 1);
}

TEST(UniquePtrTest, MultipleInheritanceToSecondBase) {
    DefaultAllocator allocator;
    SecondBase::reset_counter();
    {
        // 派生クラスのポインタを第二基底クラスのUniquePtrに移動
        UniquePtr<SecondBase> second_ptr = make_unique<DerivedClass>(&allocator, 42, 24, 10);
        EXPECT_EQ(SecondBase::get_counter(), 0);
        EXPECT_EQ(second_ptr->get_value2(), 24);
    }
    // スコープを抜けたときに仮想デストラクタが正しく呼ばれることを確認
    EXPECT_EQ(SecondBase::get_counter(), 1);
}

TEST(UniquePtrTest, MemberSwap) {
    DefaultAllocator allocator;
    auto ptr1 = make_unique<TestClass>(&allocator, 42);
    auto ptr2 = make_unique<TestClass>(&allocator, 24);

    // swap前の値を確認
    EXPECT_EQ(ptr1->get_value(), 42);
    EXPECT_EQ(ptr2->get_value(), 24);

    // メンバ関数のswapを実行
    ptr1.swap(ptr2);

    // swap後の値を確認
    EXPECT_EQ(ptr1->get_value(), 24);
    EXPECT_EQ(ptr2->get_value(), 42);
}

TEST(UniquePtrTest, GlobalSwap) {
    DefaultAllocator allocator;
    auto ptr1 = make_unique<TestClass>(&allocator, 42);
    auto ptr2 = make_unique<TestClass>(&allocator, 24);

    // swap前の値を確認
    EXPECT_EQ(ptr1->get_value(), 42);
    EXPECT_EQ(ptr2->get_value(), 24);

    // グローバル関数のswapを実行
    swap(ptr1, ptr2);

    // swap後の値を確認
    EXPECT_EQ(ptr1->get_value(), 24);
    EXPECT_EQ(ptr2->get_value(), 42);
}

TEST(UniquePtrTest, SwapWithEmpty) {
    DefaultAllocator allocator;
    auto ptr1 = make_unique<TestClass>(&allocator, 42);
    UniquePtr<TestClass> ptr2;

    // swap前の状態を確認
    EXPECT_TRUE(ptr1);
    EXPECT_FALSE(ptr2);
    EXPECT_EQ(ptr1->get_value(), 42);

    // swapを実行
    ptr1.swap(ptr2);

    // swap後の状態を確認
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->get_value(), 42);
}

TEST(UniquePtrTest, ArrayDefaultConstruction) {
    DefaultAllocator allocator;
    constexpr std::size_t size = 5;
    auto arr = make_unique<TestClass[]>(&allocator, size);

    EXPECT_TRUE(arr);
    EXPECT_EQ(arr.length(), size);

    // デフォルト構築された要素の値を確認
    for (std::size_t i = 0; i < size; ++i) {
        EXPECT_EQ(arr[i].get_value(), 0);
    }
}

TEST(UniquePtrTest, ArrayModification) {
    DefaultAllocator allocator;
    constexpr std::size_t size = 3;
    auto arr = make_unique<TestClass[]>(&allocator, size);

    // 配列の要素を変更
    for (std::size_t i = 0; i < size; ++i) {
        arr[i].set_value(static_cast<int>(i * 10));
    }

    // 変更後の値を確認
    for (std::size_t i = 0; i < size; ++i) {
        EXPECT_EQ(arr[i].get_value(), static_cast<int>(i * 10));
    }
}

TEST(UniquePtrTest, ArrayDestructorCall) {
    DefaultAllocator allocator;
    constexpr std::size_t size = 4;
    DestructorTracker::reset_counter();
    {
        auto arr = make_unique<DestructorTracker[]>(&allocator, size);
        EXPECT_EQ(DestructorTracker::get_counter(), 0);
    }
    // 配列の全要素のデストラクタが呼ばれることを確認
    EXPECT_EQ(DestructorTracker::get_counter(), size);
}

TEST(UniquePtrTest, ArrayMoveConstruction) {
    DefaultAllocator allocator;
    constexpr std::size_t size = 3;
    auto arr1 = make_unique<TestClass[]>(&allocator, size);

    // 要素を設定
    for (std::size_t i = 0; i < size; ++i) {
        arr1[i].set_value(static_cast<int>(i * 10));
    }

    // ムーブ構築
    UniquePtr<TestClass[]> arr2(std::move(arr1));

    // 元のポインタがnullptrになることを確認
    EXPECT_FALSE(arr1); // NOLINT
    EXPECT_EQ(arr1.length(), 0);

    // 移動先の状態を確認
    EXPECT_TRUE(arr2);
    EXPECT_EQ(arr2.length(), size);
    for (std::size_t i = 0; i < size; ++i) {
        EXPECT_EQ(arr2[i].get_value(), static_cast<int>(i * 10));
    }
}

TEST(UniquePtrTest, ArrayMoveAssignment) {
    DefaultAllocator allocator;
    constexpr std::size_t size = 3;
    auto arr1 = make_unique<TestClass[]>(&allocator, size);
    auto arr2 = make_unique<TestClass[]>(&allocator, 2); // 異なるサイズの配列

    // 要素を設定
    for (std::size_t i = 0; i < size; ++i) {
        arr1[i].set_value(static_cast<int>(i * 10));
    }

    // ムーブ代入
    arr2 = std::move(arr1);

    // 元のポインタがnullptrになることを確認
    EXPECT_FALSE(arr1); // NOLINT
    EXPECT_EQ(arr1.length(), 0);

    // 移動先の状態を確認
    EXPECT_TRUE(arr2);
    EXPECT_EQ(arr2.length(), size);
    for (std::size_t i = 0; i < size; ++i) {
        EXPECT_EQ(arr2[i].get_value(), static_cast<int>(i * 10));
    }
}

} // namespace
} // namespace w6_mem
