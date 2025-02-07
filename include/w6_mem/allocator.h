#pragma once

#include <cstddef>

namespace w6_mem {

class IAllocator {
  public:
    virtual ~IAllocator() = default;

    virtual void *allocate(std::size_t size, std::size_t alignment) = 0;
    virtual void deallocate(void *ptr) = 0;
};

class DefaultAllocator : public IAllocator {
  public:
    void *allocate(std::size_t size, std::size_t alignment) override {
#if defined(_MSC_VER)
        return _aligned_malloc(size, alignment);
#else
        return std::aligned_alloc(alignment, size);
#endif
    }

    void deallocate(void *ptr) override {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }
};

} // namespace w6_mem
