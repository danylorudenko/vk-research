#pragma once

#include <foundation/Common.hpp>
#include <memory/Pointer.hpp>

DRE_BEGIN_NAMESPACE


/*
*
* InplaceVector is a wrapper around an inplace array and size variable.
* Internal storage has compile-time size, though data in storage may vary in count.
*
* This container can't expand if no more storage left for data.
* Elements are stored in sequedential order.
* WARNING: upon element removal order is not preserved.
*
* StackVector doesn't have move operations since those are considered non-optimal, but may be added later.
* To copy data it is advised to use external functions.
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
*   + StorageSize       ()
*   + Data              (index = 0)
*   + operator[]        (index)
*
*/
template<typename T, U32 STORAGE_SIZE>
class InplaceVector
{
    static_assert(STORAGE_SIZE != 0, "InplaceVector storage size can't be ZERO.");

public:
    inline InplaceVector()
        : m_Size{ 0 }
    {
    }

    inline ~InplaceVector()
    {
        U32 const count = m_Size;
        for (U32 i = 0; i < count; ++i)
        {
            Data()[i].~T();
        }
    }


    InplaceVector(InplaceVector<T, STORAGE_SIZE>&& rhs)
    {
        operator=(DRE_MOVE(rhs));
    }

    InplaceVector<T, STORAGE_SIZE>& operator=(InplaceVector<T, STORAGE_SIZE>&& rhs)
    {
        DRE_ASSERT(m_Size == 0, "Can't move into non-empty InplaceVector.");
        
        U32 const count = Size();
        for (U32 i = 0; i < count; ++i)
        {
            m_Storage[i] = DRE_MOVE(rhs.m_Storage[i]);
        }

        m_Size = rhs.m_Size;
        rhs.m_Size = 0;

        return *this;
    }

    inline T& operator[](U32 index)
    {
        DRE_ASSERT(index < m_Size, "InplaceVector: out of bounds!");
        return Data()[index];
    }

    inline T const& operator[](U32 index) const
    {
        DRE_ASSERT(index < m_Size, "InplaceVector: out of bounds!");
        return Data()[index];
    }

    void Clear()
    {
        U32 const count = m_Size;
        for (U32 i = 0; i < count; ++i)
        {
            Data()[i].~T();
        }

        m_Size = 0;
    }

    inline T* Data()
    {
        return (T*)m_Storage;
    }

    inline T const* Data() const
    {
        return (T*)m_Storage;
    }

    inline U32 Size() const
    {
        return m_Size;
    }

    U32 constexpr StorageSize() const
    {
        return STORAGE_SIZE * sizeof(T);
    }

    template<typename... TArgs>
    inline T& EmplaceBack(TArgs&&... args)
    {
        DRE_ASSERT(m_Size + 1 <= STORAGE_SIZE, "InplaceVector: out of storage!");

        return *(new (Data() + m_Size++) T{ std::forward<TArgs>(args)... });
    }

    inline void Remove(T* element)
    {
        DRE_ASSERT(PtrDifference(element, m_Storage) >= 0, "InplaceVector::Remove out of bounds!");
        DRE_ASSERT(PtrDifference(Data() + m_Size, element) > 0, "InplaceVector::Remove out of bounds!");

        U32 const index = (U32)(PtrDifference(element, m_Storage) / sizeof(T));
        RemoveIndex(index);
    }

    inline void RemoveIndex(U32 index)
    {
        DRE_ASSERT(index < m_Size, "InplaceVector::RemoveIndex out of bounds!");

        if (index == m_Size - 1) // last element
        {
            Data()[index].~T();
        }
        else
        {
            Data()[index].~T();
            new (Data() + index) T{ std::move(Data()[m_Size - 1]) };
        }

        m_Size--;
    }

private:
    alignas(alignof(T))
    U8  m_Storage[STORAGE_SIZE * sizeof(T)];
    U32 m_Size;
};

DRE_END_NAMESPACE

