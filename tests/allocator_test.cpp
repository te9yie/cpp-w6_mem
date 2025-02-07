#include <cstring>
#include <gtest/gtest.h>
#include <w6_mem/allocator.h>

namespace {

TEST(DefaultAllocatorTest, AllocateAndDeallocate) {
    w6_mem::DefaultAllocator allocator;

    // 基本的なアロケーションとデアロケーションのテスト
    constexpr std::size_t SIZE = 1024;
    constexpr std::size_t ALIGNMENT = 16;

    void *ptr = allocator.allocate(SIZE, ALIGNMENT);
    ASSERT_NE(ptr, nullptr);

    // アラインメントのチェック
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(ptr) % ALIGNMENT, 0);

    // メモリに書き込みができることを確認
    std::memset(ptr, 0xFF, SIZE);

    // デアロケーション
    allocator.deallocate(ptr);
}

TEST(DefaultAllocatorTest, MultipleAllocations) {
    w6_mem::DefaultAllocator allocator;

    constexpr int NUM_ALLOCATIONS = 100;
    constexpr std::size_t SIZE = 64;
    constexpr std::size_t ALIGNMENT = 32;

    std::vector<void *> ptrs;
    ptrs.reserve(NUM_ALLOCATIONS);

    // 複数のアロケーションを実行
    for (int i = 0; i < NUM_ALLOCATIONS; ++i) {
        void *ptr = allocator.allocate(SIZE, ALIGNMENT);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(ptr) % ALIGNMENT, 0);
        ptrs.push_back(ptr);
    }

    // すべてのメモリを解放
    for (void *ptr : ptrs) {
        allocator.deallocate(ptr);
    }
}

TEST(DefaultAllocatorTest, DifferentSizesAndAlignments) {
    w6_mem::DefaultAllocator allocator;

    struct TestCase {
        std::size_t size{};
        std::size_t alignment{};
    };

    std::vector<TestCase> test_cases = {{1, 1},   {8, 8},     {16, 16},   {32, 32},
                                        {64, 64}, {128, 128}, {1024, 256}};

    for (const auto &test_case : test_cases) {
        void *ptr = allocator.allocate(test_case.size, test_case.alignment);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(ptr) % test_case.alignment, 0);
        allocator.deallocate(ptr);
    }
}

} // namespace
