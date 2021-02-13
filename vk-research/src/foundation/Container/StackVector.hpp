#pragma once

#include <foundation\Common.hpp>
#include <memory\AllocatorScopeStack.hpp>


DRE_BEGIN_NAMESPACE


/*
*
* StackVector class based on AllocatorScopeStack and it's AllocationScopes and designed to be used with them.
* Can't operate without AllocationScope from AllocatorScopeStack.
* 
* Overall behaves like STD vector, relying on storage expansion on need(when no storage left, requests size*2).
* Elements are stored in sequedential order, like an array.
* ONE MAJOR DIFFERENCE from STD: upon element removal order is not preserved.
*
* StackVector doesn't have move operations since those would bare potential woulnerabilities.
* Move operations would also violate principal "AllocationScopes are not moved".
*
*
* Basic interface:
*
*   + EmplaceBack       (args...)
*
*   + Remove            (T* element)
*   + RemoveIndex       (index)
*
*   + Size              ()
*   + ReservedSize      ()
*   + Data              (index = 0)
*   + operator[]        (index)
*
*/
template<typename T>
class StackVector
{
public:
    StackVector(AllocationScope* allocator)
        : m_Allocator   { allocator }
        , m_Data        { nullptr }
        , m_ReservedSize{ 0 }
        , m_Size        { 0 }
    {
    }

    StackVector(AllocationScope* allocator, U32 size)
        : m_Allocator   { allocator }
        , m_Data        { nullptr }
        , m_ReservedSize{ 0 }
        , m_Size        { 0 }
    {
        m_Data = (T*)allocator->Alloc(sizeof(T) * size, alignof(T));
        for (U32 i = 0; i < size; ++i)
        {
            new (m_Data + i) T{};
        }
        m_ReservedSize = size;
        m_Size = size;
    }

    ~StackVector()
    {
        Clear();
    }

    T* Data()
    {
        return m_Data;
    }

    T const* Data() const
    {
        return m_Data;
    }

    T const& operator[](U32 index) const
    {
        return m_Data[index];
    }

    T& operator[](U32 index)
    {
        return m_Data[index];
    }

    U32 Size() const
    {
        return m_Size;
    }

    U32 ReservedSize() const
    {
        return m_ReservedSize;
    }

    template<typename... TArgs>
    T& EmplaceBack(TArgs&&... args)
    {
        if ((m_Size + 1) > m_ReservedSize)
        {
            U32 targetReservedSize = 0;
            if (m_ReservedSize == 0)
            {
                targetReservedSize = 8;
            }
            else
            {
                targetReservedSize = m_ReservedSize * 2;
            }
            
            T* newData = (T*)m_Allocator->Alloc(sizeof(T) * targetReservedSize, alignof(T));
            m_ReservedSize = targetReservedSize;

            U32 const count = m_Size;
            for (U32 i = 0; i < count; ++i)
            {
                newData[i] = std::move(m_Data[i]);
            }

            m_Data = newData;
        }

        return *(new (m_Data + (m_Size++)) T{ std::forward<TArgs>(args)... });
    }

    void RemoveIndex(U32 index)
    {
        DRE_ASSERT(index < m_Size, "StackVector::RemoveIndex out of bounds!");

        if (index == m_Size - 1) // last element
        {
            m_Data[index].~T();
        }
        else
        {
            m_Data[index].~T();
            new (m_Data + index) T(std::move(m_Data[m_Size - 1]));
        }

        --m_Size;
    }

    void Remove(T* element)
    {
        DRE_ASSERT(m_Data != nullptr, "StackVector::Remove on empty!");
        DRE_ASSERT(PtrDifference(element, m_Data) >= 0, "StackVector::Remove out of bounds!");
        DRE_ASSERT(PtrDifference(m_Data + m_Size, element) > 0, "StaclVector::Remove out of bounds!");

        U32 const index = (U32)(PtrDifference(element, m_Data) / sizeof(T));
        RemoveIndex(index);
    }

    void Clear()
    {
        U32 const count = m_Size;
        for (U32 i = 0; i < count; ++i)
        {
            m_Data[i].~T();
        }

        m_Size = 0;
    }

private:
    AllocationScope* m_Allocator;

    T*  m_Data;
    U32 m_ReservedSize;
    U32 m_Size;
};

DRE_END_NAMESPACE

