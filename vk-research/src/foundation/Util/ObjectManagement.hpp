#pragma once

#include <foundation\Common.hpp>

DRE_BEGIN_NAMESPACE

template<typename T>
class UniquePlacedPtr
{
public:
    UniquePlacedPtr() 
        : m_Ptr{ nullptr } 
    {}

    UniquePlacedPtr(UniquePlacedPtr&& rhs) 
        : m_Ptr{ nullptr } 
    { 
        Swap(DRE_MOVE(rhs));
    }

    UniquePlacedPtr& operator=(UniquePlacedPtr&& rhs)
    {
        Swap(DRE_MOVE(rhs));

        return *this;
    }

    UniquePlacedPtr& operator=(std::nullptr_t)
    {
        DestructObj();

        return *this;
    }

    T& operator*()
    {
        return *m_Ptr;
    }

    T const& operator*() const
    {
        return *m_Ptr;
    }

    T* operator->()
    {
        return m_Ptr;
    }

    T const* operator->() const
    {
        return m_Ptr;
    }

    ~UniquePlacedPtr()
    {
        DestructObj();
    }

    void Reset(T* ptr)
    {
        DestructObj();
        m_Ptr = ptr;
    }

    UniquePlacedPtr(UniquePlacedPtr const&) = delete;
    UniquePlacedPtr& operator=(UniquePlacedPtr const&) = delete;

private:
    inline void Swap(UniquePlacedPtr&& rhs)
    {
        //DRE_SWAP(m_Ptr, rhs.m_Ptr);
        std::swap(m_Ptr, rhs.m_Ptr);
    }

    inline void DestructObj()
    {
        if (m_Ptr != nullptr)
            m_Ptr->~T();

        m_Ptr = nullptr;
    }

private:
    T* m_Ptr;
};

DRE_END_NAMESPACE

