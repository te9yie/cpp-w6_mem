#include "w6_mem/allocator.h"
#include "w6_mem/stl_allocator.h"
#include <algorithm>
#include <cstdlib>
#include <gtest/gtest.h>
#include <vector>

// DummyAllocator は IAllocator の簡易実装です。
class DummyAllocator : public w6_mem::IAllocator {
public:
    std::vector<void*> allocations;

    /**
     * @brief 指定されたサイズのメモリを確保します。
     *
     * @param size 確保するバイト数
     * @param alignment アライメント値（未使用）
     * @return void* 確保されたメモリへのポインタ
     */
    void* allocate(std::size_t size, std::size_t alignment) override {
        void* ptr = std::malloc(size);
        allocations.push_back(ptr);
        return ptr;
    }

    /**
     * @brief 指定されたメモリを解放します。
     *
     * @param ptr 解放するメモリへのポインタ
     */
    void deallocate(void* ptr) override {
        auto it = std::find(allocations.begin(), allocations.end(), ptr);
        if (it != allocations.end()) {
            allocations.erase(it);
            std::free(ptr);
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
