#pragma once

#include <foundation\Common.hpp>
#include <memory\Pointer.hpp>
#include <math\SimpleMath.hpp>

DRE_BEGIN_NAMESPACE

/*
*
* Linear allocation implementation.
* Free unimplemented
*
*
* Basic interface:
*
*   + Alloc (size, alignment)
*   + Free  ()                  // this is an empty method
*   + Reset ()
*   + MemorySize()
*
*/
class AllocatorLinear
{
public:
    inline void* Alloc(U64 size, U32 alignment)
    {
        DRE_ASSERT(IsValidAlignment(alignment), "Allocator<AllocStartLiner, ...>: received invalid allocation alignment.");
        DRE_ASSERT(size + alignment <= FreeSize(), "Allocator<AllocStratLinear, ...>: no more memory in allocator.");

        void* result_start = m_NextFree;
        m_NextFree = PtrAdd(m_NextFree, PtrDiff(size + alignment));

        return PtrAlign(result_start, alignment);
    }

    template<typename T, typename... TArgs>
    inline T* AllocClass(TArgs&&... args)
    {
        void* mem = Alloc(sizeof(T), alignof(T));
        return new (mem) T{ std::forward<TArgs>(args)... };
    }

    inline U64 MemorySize() const
    {
        return m_Size;
    }

    inline void Reset()
    {
        m_NextFree = m_Memory;
    }


    // Scope stacks support
    inline void ResetPosition(void* resetPoint)
    {
        m_NextFree = resetPoint;
    }

    inline void* CurrentPosition() const
    {
        return m_NextFree;
    }



    AllocatorLinear()
        : m_Memory{ nullptr }
        , m_Size{ 0 }
        , m_NextFree{ nullptr }
    {}

    AllocatorLinear(void* memory, U64 size)
        : m_Memory{ memory }
        , m_Size{ size }
        , m_NextFree{ memory }
    {
        DRE_ASSERT(m_Memory != nullptr, "AllocatorLinear: received null memory.");
        DRE_ASSERT(m_Size != 0, "AllocatorLiner: received null size.");
    }

    AllocatorLinear(AllocatorLinear&& rhs)
    {
        operator=(DRE_MOVE(rhs));
    }

    AllocatorLinear& operator=(AllocatorLinear&& rhs)
    {
        DRE_ASSERT(m_Memory == nullptr, "Can't move-assign to initialized allocators.");
        m_Memory = rhs.m_Memory;            rhs.m_Memory = nullptr;
        m_Size = rhs.m_Size;                rhs.m_Size = 0;
        m_NextFree = rhs.m_NextFree;        rhs.m_NextFree = nullptr;

        return *this;
    }

    AllocatorLinear(AllocatorLinear const& rhs) = delete;
    AllocatorLinear& operator=(AllocatorLinear const& rhs) = delete;

    ~AllocatorLinear()
    {
        m_Memory = nullptr;
        m_Size = 0;
        m_NextFree = nullptr;
    }


private:
    inline U64 AllocatedSize() const
    {
        return (U64)PtrDifference(m_NextFree, m_Memory);
    }

    inline U64 FreeSize() const
    {
        return m_Size - AllocatedSize();
    }

    inline static bool IsValidAlignment(U32 alignment)
    {
        return IsPowOf2(alignment);
    }

private:
    // Generic allocator section
    void*       m_Memory;
    U64         m_Size;

    // Linear allocator section
    void*       m_NextFree;
};

DRE_END_NAMESPACE

