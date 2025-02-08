#include "w6_mem/allocator.h"
#include "w6_mem/stl_allocator.h"
#include <algorithm>
#include <cstdlib>
#include <gtest/gtest.h>
#include <list>
#include <vector>

// DummyAllocator は IAllocator の簡易実装です。
class DummyAllocator : public w6_mem::DefaultAllocator {
public:
    std::vector<void*> m_allocations;

    /**
     * @brief 指定されたサイズのメモリを確保します。
     *
     * @param size 確保するバイト数
     * @param alignment アライメント値（未使用）
     * @return void* 確保されたメモリへのポインタ
     */
    void* allocate(std::size_t size, std::size_t alignment) override {
        void* ptr = w6_mem::DefaultAllocator::allocate(size, alignment);
        m_allocations.push_back(ptr);
        return ptr;
    }

    /**
     * @brief 指定されたメモリを解放します。
     *
     * @param ptr 解放するメモリへのポインタ
     */
    void deallocate(void* ptr) override {
        auto it = std::find(m_allocations.begin(), m_allocations.end(), ptr);
        if (it != m_allocations.end()) {
            m_allocations.erase(it);
            w6_mem::DefaultAllocator::deallocate(ptr);
        }
    }
};

// StlAllocator の allocate() が非ゼロサイズの場合、nullptr以外のポインタを返すことを確認します。
TEST(StlAllocatorTest, AllocationNonZero) {
    DummyAllocator dummy;
    w6_mem::StlAllocator<int> allocator(&dummy);
    int* p = allocator.allocate(10);
    EXPECT_NE(p, nullptr);
    allocator.deallocate(p, 10);
}

// StlAllocator の allocate() がゼロサイズの場合、nullptrを返すことを確認します。
TEST(StlAllocatorTest, AllocationZero) {
    DummyAllocator dummy;
    w6_mem::StlAllocator<int> allocator(&dummy);
    int* p = allocator.allocate(0);
    EXPECT_EQ(p, nullptr);
}

// get_allocator() が正しいポインタを返すことを確認します。
TEST(StlAllocatorTest, GetAllocator) {
    DummyAllocator dummy;
    w6_mem::StlAllocator<int> allocator(&dummy);
    EXPECT_EQ(allocator.get_allocator(), &dummy);
}

// 等価演算子 (==, !=) の動作を確認します。
TEST(StlAllocatorTest, EqualityOperators) {
    DummyAllocator dummy1;
    DummyAllocator dummy2;
    w6_mem::StlAllocator<int> allocator1(&dummy1);
    w6_mem::StlAllocator<int> allocator2(&dummy1);
    w6_mem::StlAllocator<int> allocator3(&dummy2);

    EXPECT_TRUE(allocator1 == allocator2);
    EXPECT_FALSE(allocator1 == allocator3);
    EXPECT_TRUE(allocator1 != allocator3);
}

// std::listでStlAllocatorを利用できるかのテスト
TEST(StdListStlAllocatorTest, BasicOperations) {
    DummyAllocator dummy;
    // DummyAllocatorを使ってStlAllocatorを生成します。
    w6_mem::StlAllocator<int> allocator(&dummy);

    // StlAllocatorを使用するstd::listを生成します。
    std::list<int, w6_mem::StlAllocator<int>> myList(allocator);

    // 要素を追加する。
    myList.push_back(1);
    myList.push_back(2);
    myList.push_back(3);

    // リストのサイズを確認
    EXPECT_EQ(myList.size(), 3u);

    // 要素の順序確認
    auto it = myList.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);

    // 先頭要素を削除し、残りのサイズと先頭要素を確認
    myList.pop_front();
    EXPECT_EQ(myList.size(), 2u);
    EXPECT_EQ(myList.front(), 2);
}
