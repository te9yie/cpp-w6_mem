#pragma once

#include <cstddef>
#include <cstdlib>

namespace w6_mem {

/**
 * @brief メモリアロケーション用のインターフェース
 *
 * このクラスは、動的メモリアロケーションのための基本的な関数を定義します。
 */
class IAllocator {
public:
    /**
     * @brief 仮想デストラクタ
     */
    virtual ~IAllocator() = default;

    /**
     * @brief 指定されたサイズのメモリを、指定されたアライメントで割り当てます。
     *
     * @param size 割り当てるメモリのバイト数
     * @param alignment 必要なアライメント値
     * @return void* 割り当てられたメモリへのポインタ
     */
    virtual void* allocate(std::size_t size, std::size_t alignment) = 0;

    /**
     * @brief 指定されたメモリを解放します。
     *
     * @param ptr 解放するメモリへのポインタ
     */
    virtual void deallocate(void* ptr) = 0;
};

/**
 * @brief IAllocatorのデフォルト実装
 *
 * このクラスは、システム標準のメモリアロケーション関数（std::aligned_alloc, std::aligned_free
 * 等）を利用して、 メモリの割り当てと解放を行います。
 */
class DefaultAllocator : public IAllocator {
public:
    /**
     * @brief メモリを割り当てます。
     *
     * @param size 割り当てるサイズ（バイト単位）
     * @param alignment 必要なアライメント値
     * @return void* 割り当てられたメモリへのポインタ
     */
    void* allocate(std::size_t size, std::size_t alignment) override {
#if defined(_MSC_VER)
        return _aligned_malloc(size, alignment);
#else
        return std::aligned_alloc(alignment, size);
#endif
    }

    /**
     * @brief メモリを解放します。
     *
     * @param ptr 解放するメモリへのポインタ
     */
    void deallocate(void* ptr) override {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }
};

} // namespace w6_mem
