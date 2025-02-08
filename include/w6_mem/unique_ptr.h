#pragma once

#include "allocator.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace w6_mem {

/**
 * @brief カスタムアロケータ対応のユニークポインタ（単一オブジェクト版）
 * @tparam T 管理対象の型
 */
template <typename T>
class UniquePtr {
private:
    // コピー禁止
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // 派生クラスからのアクセスを許可
    template <typename U>
    friend class UniquePtr;

public:
    using element_type = T;
    using pointer = T*;

public:
    /**
     * @brief デフォルトコンストラクタ
     * 初期状態では空のポインタとアロケータを持ちます。
     */
    constexpr UniquePtr() : m_allocator(nullptr), m_ptr(nullptr), m_allocated_memory(nullptr) {}

    /**
     * @brief nullptrからの構築
     */
    constexpr UniquePtr(std::nullptr_t) : UniquePtr() {}

    /**
     * @brief ポインタとアロケータからの構築
     * @param allocator 使用するアロケータ
     * @param p 管理対象のオブジェクトへのポインタ
     * @param allocated_memory アロケータが割り当てたメモリブロック
     */
    UniquePtr(IAllocator* allocator, pointer p, void* allocated_memory)
        : m_allocator(allocator), m_ptr(p), m_allocated_memory(allocated_memory) {}

    /**
     * @brief 派生クラスから基底クラスへのムーブ変換コンストラクタ
     * @tparam U 変換可能な型
     * @param other ムーブするUniquePtr
     */
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UniquePtr(UniquePtr<U>&& other)
        : m_allocator(other.m_allocator), m_ptr(static_cast<T*>(other.m_ptr)),
          m_allocated_memory(other.m_allocated_memory) {
        other.m_allocator = nullptr;
        other.m_ptr = nullptr;
        other.m_allocated_memory = nullptr;
    }

    /**
     * @brief ムーブコンストラクタ
     */
    UniquePtr(UniquePtr&& other)
        : m_allocator(nullptr), m_ptr(nullptr), m_allocated_memory(nullptr) {
        swap(other);
    }

    /**
     * @brief デストラクタ
     */
    ~UniquePtr() {
        if (m_ptr && m_allocator) {
            std::destroy_at(m_ptr);
            m_allocator->deallocate(m_allocated_memory);
        }
    }

    /**
     * @brief 管理対象オブジェクトへの参照を返します。
     * @return T& 管理対象オブジェクト
     */
    T& operator*() const {
        assert(m_ptr);
        return *m_ptr; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    }

    /**
     * @brief 管理対象オブジェクトへのポインタを返します。
     * @return pointer 管理対象オブジェクトへのポインタ
     */
    pointer operator->() const {
        return m_ptr;
    }

    /**
     * @brief 管理対象オブジェクトへのポインタを取得します。
     * @return pointer 管理対象オブジェクトへのポインタ
     */
    pointer get() const {
        return m_ptr;
    }

    /**
     * @brief ユニークポインタが有効かどうかを評価します。
     * @return true オブジェクトが管理されている場合
     * @return false 管理されていない場合
     */
    explicit operator bool() const {
        return !!m_ptr;
    }

    /**
     * @brief ムーブ代入演算子
     * 他のUniquePtrからリソースを奪取して自身に割り当てます。
     * @param other ムーブするUniquePtr
     * @return UniquePtr& 自身への参照
     */
    UniquePtr& operator=(UniquePtr&& other) {
        UniquePtr(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * @brief 他のUniquePtrと管理内容を交換します。
     * @param other 入れ替える相手のUniquePtr
     */
    void swap(UniquePtr& other) {
        using std::swap;
        swap(m_allocator, other.m_allocator);
        swap(m_ptr, other.m_ptr);
        swap(m_allocated_memory, other.m_allocated_memory);
    }

private:
    IAllocator* m_allocator = nullptr;
    pointer m_ptr = nullptr;
    void* m_allocated_memory = nullptr;
};

/**
 * @brief カスタムアロケータ対応のユニークポインタ（配列版）
 * @tparam T 配列要素の型
 */
template <typename T>
class UniquePtr<T[]> {
private:
    // コピー禁止
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

public:
    using element_type = T;
    using pointer = T*;

public:
    /**
     * @brief デフォルトコンストラクタ（配列版）
     */
    constexpr UniquePtr()
        : m_allocator(nullptr), m_ptr(nullptr), m_allocated_memory(nullptr), m_length(0) {}

    /**
     * @brief nullptrからの構築（配列版）
     */
    constexpr UniquePtr(std::nullptr_t) : UniquePtr() {}

    /**
     * @brief ポインタ、メモリブロック、アロケータ、およびサイズから構築（配列版）
     * @param allocator 使用するアロケータ
     * @param ptr 配列の先頭ポインタ
     * @param allocated_memory アロケータが割り当てたメモリブロック
     * @param length 配列の要素数
     */
    UniquePtr(IAllocator* allocator, pointer ptr, void* allocated_memory, size_t length)
        : m_allocator(allocator), m_ptr(ptr), m_allocated_memory(allocated_memory),
          m_length(length) {}

    /**
     * @brief ムーブコンストラクタ（配列版）
     * 他のUniquePtrからリソースを奪取します。
     */
    UniquePtr(UniquePtr&& other)
        : m_allocator(other.m_allocator), m_ptr(other.m_ptr),
          m_allocated_memory(other.m_allocated_memory), m_length(other.m_length) {
        other.m_allocator = nullptr;
        other.m_ptr = nullptr;
        other.m_allocated_memory = nullptr;
        other.m_length = 0;
    }

    /**
     * @brief ムーブ代入演算子（配列版）
     * 他のUniquePtrからリソースを奪取して自身に割り当てます。
     * @param other ムーブするUniquePtr
     * @return UniquePtr& 自身への参照
     */
    UniquePtr& operator=(UniquePtr&& other) {
        UniquePtr(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * @brief デストラクタ（配列版）
     * 管理している配列を破棄し、アロケータを利用してメモリを解放します。
     */
    ~UniquePtr() {
        if (m_ptr && m_allocator) {
            std::destroy(m_ptr, m_ptr + m_length);
            m_allocator->deallocate(m_allocated_memory);
        }
    }

    /**
     * @brief 指定されたインデックスの配列要素への参照を返します。
     * @param index 要素のインデックス
     * @return T& 配列要素への参照
     */
    T& operator[](size_t index) {
        assert(m_ptr);
        assert(index < m_length);
        return m_ptr[index]; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    }

    /**
     * @brief 指定されたインデックスの配列要素への定数参照を返します。
     * @param index 要素のインデックス
     * @return const T& 配列要素への定数参照
     */
    const T& operator[](size_t index) const {
        assert(m_ptr);
        assert(index < m_length);
        return m_ptr[index]; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    }

    /**
     * @brief 管理している配列の先頭ポインタを取得します。
     * @return T* 配列の先頭ポインタ
     */
    T* get() const {
        return m_ptr;
    }

    /**
     * @brief 配列のサイズを返します。
     * @return size_t 配列の要素数
     */
    size_t length() const {
        return m_length;
    }

    /**
     * @brief ユニークポインタ（配列版）が有効かどうかを評価します。
     * @return true オブジェクトが管理されている場合
     * @return false 管理されていない場合
     */
    explicit operator bool() const {
        return !!m_ptr;
    }

    /**
     * @brief 他のUniquePtr（配列版）と管理内容を入れ替えます。
     * @param other 入れ替える相手のUniquePtr
     */
    void swap(UniquePtr& other) {
        using std::swap;
        swap(m_allocator, other.m_allocator);
        swap(m_ptr, other.m_ptr);
        swap(m_allocated_memory, other.m_allocated_memory);
        swap(m_length, other.m_length);
    }

private:
    IAllocator* m_allocator = nullptr;
    pointer m_ptr = nullptr;
    void* m_allocated_memory = nullptr;
    size_t m_length = 0;
};

/**
 * @brief 2つのUniquePtrの内容を交換する
 * @tparam T UniquePtrが管理する型
 * @param lhs 1つ目のUniquePtr
 * @param rhs 2つ目のUniquePtr
 */
template <typename T>
inline void swap(UniquePtr<T>& lhs, UniquePtr<T>& rhs) {
    lhs.swap(rhs);
}

template <typename T>
struct MakeUnique {
    using Single = UniquePtr<T>;
};

template <typename T>
struct MakeUnique<T[]> {
    using Array = UniquePtr<T[]>;
};

/**
 * @brief UniquePtrを作成するヘルパー関数（単一オブジェクト版）
 * @tparam T 作成する型
 * @tparam Args コンストラクタ引数の型
 * @param allocator 使用するアロケータ
 * @param args コンストラクタ引数
 * @return UniquePtr<T> 作成されたユニークポインタ
 */
template <typename T, typename... Args>
inline typename MakeUnique<T>::Single make_unique(IAllocator* allocator, Args&&... args) {
    if (!allocator) {
        return UniquePtr<T>();
    }

    void* memory = allocator->allocate(sizeof(T), alignof(T));
    if (!memory) {
        return UniquePtr<T>();
    }

    T* object = new (memory) T(std::forward<Args>(args)...);
    return UniquePtr<T>(allocator, object, memory);
}

/**
 * @brief UniquePtrを作成するヘルパー関数（配列版）
 * @tparam Ts 配列型（例：T[]）
 * @param allocator 使用するアロケータ
 * @param length 配列のサイズ
 * @return UniquePtr<Ts> 作成されたユニークポインタ（配列版）
 * @note 配列型のテンプレート引数を受け取り、その要素型の配列を構築します
 */
template <typename Ts>
inline typename MakeUnique<Ts>::Array make_unique(IAllocator* allocator, size_t length) {
    if (!allocator) {
        return UniquePtr<Ts>();
    }

    using T = std::remove_extent_t<Ts>;
    void* memory = allocator->allocate(sizeof(T) * length, alignof(T));
    if (!memory) {
        return UniquePtr<Ts>();
    }

    // 配列全体を一度に構築
    T* ptr = new (memory) T[length]();
    return UniquePtr<Ts>(allocator, ptr, memory, length);
}

} // namespace w6_mem
