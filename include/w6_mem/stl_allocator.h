#pragma once

#include "allocator.h"
#include <cstddef>
#include <memory>
#include <type_traits>

namespace w6_mem {

/**
 * @brief STLコンテナ用のアロケータアダプタ
 * @tparam T アロケートする型
 */
template <typename T>
class StlAllocator {
public:
    // STLアロケータの要件に基づく型定義
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::false_type;

    /**
     * @brief 型の再バインド用構造体
     * @tparam U 再バインド先の型
     */
    template <typename U>
    struct rebind {
        using other = StlAllocator<U>;
    };

    /**
     * @brief デフォルトコンストラクタは削除されています。
     */
    StlAllocator() = delete;

    /**
     * @brief IAllocatorを受け取るコンストラクタ
     *
     * IAllocator*からStlAllocator<T>への暗黙変換を許可し、STLコンテナの構築時に直接利用できるようにします。
     *
     * @param allocator 内部で用いるアロケータへのポインタ
     */
    /*explicit*/ StlAllocator(IAllocator* allocator) : m_allocator(allocator) {}

    /**
     * @brief 異なる型からのコピーコンストラクタ
     *
     * @tparam U コピー元の型
     * @param other コピー元のStlAllocator
     */
    template <typename U>
    StlAllocator(const StlAllocator<U>& other) : m_allocator(other.get_allocator()) {}

    /**
     * @brief メモリの確保を行います。
     *
     * 指定された個数のT型オブジェクト用のメモリをIAllocatorを用いて確保します。
     *
     * @param n 確保するオブジェクトの個数
     * @return T* 確保されたメモリへのポインタ。nが0の場合はnullptrを返します。
     */
    T* allocate(size_type n) {
        if (n == 0) {
            return nullptr;
        }
        void* memory = m_allocator->allocate(n * sizeof(T), alignof(T));
        return static_cast<T*>(memory);
    }

    /**
     * @brief メモリの解放を行います。
     *
     * 指定されたメモリポインタに対して解放処理を実施します。
     *
     * @param p 解放するメモリへのポインタ
     * @param (unused) この引数は未使用です。
     */
    void deallocate(T* p, size_type) {
        if (p) {
            m_allocator->deallocate(p);
        }
    }

    /**
     * @brief 等価性の比較
     *
     * 内部で使用しているアロケータポインタが一致するかを比較します。
     *
     * @param other 比較対象のStlAllocator
     * @return true 同じアロケータを指している場合
     * @return false 異なるアロケータの場合
     */
    bool operator==(const StlAllocator& other) const {
        return m_allocator == other.m_allocator;
    }

    /**
     * @brief 非等価の比較
     *
     * operator== の否定です。
     *
     * @param other 比較対象のStlAllocator
     * @return true 異なる場合
     * @return false 同じ場合
     */
    bool operator!=(const StlAllocator& other) const {
        return !(*this == other);
    }

    /**
     * @brief 内部アロケータへのアクセス
     *
     * 内部に保持しているIAllocatorへのポインタを返します。
     *
     * @return IAllocator* 内部アロケータへのポインタ
     */
    IAllocator* get_allocator() const {
        return m_allocator;
    }

private:
    IAllocator* m_allocator;

    template <typename U>
    friend class StlAllocator;
};

} // namespace w6_mem
