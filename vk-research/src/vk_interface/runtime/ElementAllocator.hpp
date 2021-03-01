#include <cassert>
#include <cstdint>
#include <memory>

namespace VKW
{

template<std::uint16_t LIST_SIZE>
class FreeListElementAllocator
{
    static_assert(LIST_SIZE - 1 < UINT16_MAX);

    static constexpr std::uint16_t INVALID_ELEMENT = UINT16_MAX;

public:
    FreeListElementAllocator()
    {
        Reset();
    }

    FreeListElementAllocator(FreeListElementAllocator&& rhs)
    {
        operator=(std::move(rhs));
    }

    FreeListElementAllocator& operator=(FreeListElementAllocator&& rhs)
    {
        std::memcpy(this, &rhs, sizeof(*this));
        return *this;
    }

    std::uint16_t Allocate(std::uint16_t count)
    {
        assert(firstFree_ != INVALID_ELEMENT);
        assert(count == 1);

        std::uint16_t result = firstFree_;
        firstFree_ = storage_[firstFree_];
        return result;
    }

    void Free(std::uint16_t element)
    {
        storage_[element] = firstFree_;
        firstFree_ = element;
    }

    void Reset()
    {
        for (std::uint16_t i = 0; i < LIST_SIZE - 1; i++)
        {
            storage_[i] = i + 1;
        }
        storage_[LIST_SIZE - 1] = INVALID_ELEMENT;

        firstFree_ = 0;
    }

private:
    std::uint16_t storage_[LIST_SIZE];
    std::uint16_t firstFree_;
};


////////////////////////////////////////////////

template<std::uint16_t SIZE>
class LinearElementAllocator
{
public:
    std::uint16_t Allocate(std::uint16_t count)
    {
        assert((nextFree_ + count) < SIZE);

        std::uint16_t result = nextFree_;
        nextFree_ += count;
        return result;
    }

    void Free(std::uint16_t element) {}

    void Reset()
    {
        nextFree_ = 0;
    }

private:
    std::uint16_t nextFree_;
};

}
