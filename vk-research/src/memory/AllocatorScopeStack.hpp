#pragma once

#include <foundation\Common.hpp>
#include <memory\Pointer.hpp>

DRE_BEGIN_NAMESPACE


class AllocatorScopeStack;



/*
*
* AllocationScope class accompanying AllocatorScopeStack below.
* Source: https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/scopestacks-public.pdf
*
* It's guaranteed that allocation scope will not be moved elsewhere through it's lifetime, because it heavily relies
* on C++ stack functionality. This means all objects up in stack can safely depend on it's pointer.
* 
* Allocations from scope are directed to allocator.
* Scope stores separate finalizers for arrays and for separate objects.
* So on destruction scope automatically destructs all allocated entities, array allocations are destroyed first.
*
* Basic interface:
*
*   + Alloc             (size, alignment)
*
*   + AllocPod<T>       ()
*   + AllocPodArray<T>  ()
*
*   + AllocClass<T>     ()
*   + AllocClassArray<T>()
*
*/
class AllocationScope
{
public:
    AllocationScope(AllocatorScopeStack* allocator, void* unwindPoint, U8 level)
        : m_Allocator           { allocator }
        , m_UnwindPoint         { unwindPoint }
        , m_FinalizerChain      { nullptr }
        , m_ArrayFinalizerChain { nullptr }
        , m_Level               { level }
    {
    }


    AllocationScope(AllocationScope&& rhs)
        : m_Allocator           { rhs.m_Allocator }
        , m_UnwindPoint         { rhs.m_UnwindPoint }
        , m_FinalizerChain      { rhs.m_FinalizerChain }
        , m_ArrayFinalizerChain { rhs.m_ArrayFinalizerChain }
        , m_Level               { rhs.m_Level }
    {
        rhs.m_Allocator = nullptr;
        rhs.m_UnwindPoint = nullptr;
        rhs.m_FinalizerChain = nullptr;
        rhs.m_ArrayFinalizerChain = nullptr;
        rhs.m_Level = 0;
    }

    AllocationScope(AllocationScope const&) = delete;

    AllocationScope& operator=(AllocationScope const&) = delete;
    AllocationScope& operator=(AllocationScope&&) = delete;

    ~AllocationScope();

    void* Alloc(U64 size, U32 alignment);

    template<typename T>
    inline T* AllocClass();

    template<typename T>
    inline T* AllocClassArray(U32 count);

    template<typename T>
    inline void* AllocPod();

    template<typename T>
    inline void* AllocPodArray(U32 count);




private:
    template<typename T>
    static void DestructorCall(void* obj)
    {
        ((T*)obj)->~T();
    }

    struct Finalizer
    {
        void (*destructor)(void*);
        Finalizer* next;
        U8 objectOffset;
    };

    struct ArrayFinalizer
    {
        void (*destructor)(void*);
        ArrayFinalizer* next;
        U32 objectsCount;
        U32 objectSize;
        U8 objectsOffset;
    };


private:
    AllocatorScopeStack*    m_Allocator;
    void*                   m_UnwindPoint;
    Finalizer*              m_FinalizerChain;
    ArrayFinalizer*         m_ArrayFinalizerChain;
    U8                      m_Level;
};





/*
*
* Scope stack allocator implementation.
* Source: https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/scopestacks-public.pdf
*
* Allocation are made from AllocationScope not directly from scope stacks allocator.
*
*
* Basic public interface:
*
*   + AllocationScope PushAllocationScope()
*
*
* That's it folks, allocator only provides allocation scope through which all allocations are made.
*
*/
class AllocatorScopeStack
{
public:
    AllocatorScopeStack()
        : m_Memory  { nullptr }
        , m_Size    { 0 }
        , m_NextFree{ nullptr }
        , m_TopLevel{ 0 }
    {
    }

    AllocatorScopeStack(void* memory, U64 memorySize)
        : m_Memory  { memory }
        , m_Size    { memorySize }
        , m_NextFree{ memory }
        , m_TopLevel{ 0 }
    {
    }

    AllocatorScopeStack(AllocatorScopeStack&& rhs)
        : m_Memory  { rhs.m_Memory }
        , m_Size    { rhs.m_Size }
        , m_NextFree{ rhs.m_NextFree }
        , m_TopLevel{ rhs.m_TopLevel }
    {
        rhs.m_Memory = nullptr;
        rhs.m_Size = 0;
        rhs.m_NextFree = nullptr;
        rhs.m_TopLevel = 0;
    }

    AllocatorScopeStack& operator=(AllocatorScopeStack&& rhs)
    {
        DRE_ASSERT(m_Memory == nullptr, "Can't move-assign to initialized AllocatorScopeStack.");

        m_Memory = rhs.m_Memory;        rhs.m_Memory = nullptr;
        m_Size = rhs.m_Size;            rhs.m_Size = 0;
        m_NextFree = rhs.m_NextFree;    rhs.m_NextFree = nullptr;
        m_TopLevel = rhs.m_TopLevel;    rhs.m_TopLevel = 0;

        return *this;
    }

    AllocatorScopeStack(AllocatorScopeStack const&) = delete;
    AllocatorScopeStack& operator=(AllocatorScopeStack const&) = delete;

    ~AllocatorScopeStack()
    {
        DRE_ASSERT(m_TopLevel == 0, "AllocatorScopeStack: not all scopes were unwind before allocator destruction");
    }

    inline AllocationScope PushAllocationScope()
    {
        return AllocationScope{ this, m_NextFree, ++m_TopLevel };
    }

    inline void* Alloc(U64 size, U32 alignment, U8 level)
    {
        DRE_ASSERT(level == m_TopLevel, "AllocatorScopeStack detected allocation from the intermediate scope.");

        void* data = PtrAlign(m_NextFree, alignment);
        void* dataEnd = PtrAdd(data, (PtrDiff)size);

        DRE_ASSERT(PtrDifference(PtrAdd(m_Memory, m_Size), dataEnd) > 0, "AllocatorScopeStack: out of memory!");

        m_NextFree = dataEnd;

        return data;
    }

    template<typename FinalizerT>
    inline void* AllocWithFinalizer(FinalizerT*& finalizer, U64 size, U32 alignment, U8 level)
    {
        DRE_ASSERT(level == m_TopLevel, "AllocatorScopeStack detected allocation from the intermediate scope.");

        finalizer = PtrAlign((FinalizerT*)m_NextFree, alignof(FinalizerT));
        void* data = PtrAlign(finalizer + 1, alignment);
        void* dataEnd = PtrAdd(data, (PtrDiff)size);

        DRE_ASSERT(PtrDifference(PtrAdd(m_Memory, m_Size), dataEnd) > 0, "AllocatorScopeStack: out of memory!");

        m_NextFree = dataEnd;

        return data;
    }

    inline void Unwind(void* point, U8 level)
    {
        DRE_ASSERT(PtrDifference(point, m_Memory) >= 0,                          "AllocatorScopeStack: unwind point is out of bound");
        DRE_ASSERT(PtrDifference(PtrAdd(m_Memory, (PtrDiff)m_Size), point) > 0, "AllocatorScopeStack: unwind point is out of bound");
        DRE_ASSERT(level == m_TopLevel, "AllocatorScope stack: attempt to unwind from intermediate scope.");
        
        m_NextFree = point;
        m_TopLevel--;
    }
    
    inline U64 MemorySize() const
    {
        return m_Size;
    }

    inline U8 TopLevel() const
    {
        return m_TopLevel;
    }

    inline void* CurrentPosition() const
    {
        return m_NextFree;
    }

private:
    void*               m_Memory;
    U64                 m_Size;

    void*               m_NextFree;
    U8                  m_TopLevel;

};



template<typename T>
inline T* AllocationScope::AllocClass()
{
    Finalizer* finalizer = nullptr;

    T* data = (T*)m_Allocator->AllocWithFinalizer(finalizer, sizeof(T), alignof(T), m_Level);
    new (data) T();

    finalizer->next = m_FinalizerChain;
    finalizer->destructor = &DestructorCall<T>;
    finalizer->objectOffset = (U8)PtrDifference(data, finalizer);

    m_FinalizerChain = finalizer;

    return data;
}

template<typename T>
inline T* AllocationScope::AllocClassArray(U32 count)
{
    ArrayFinalizer* finalizer = nullptr;

    T* data = (T*)m_Allocator->AllocWithFinalizer(finalizer, sizeof(T) * count, alignof(T), m_Level);
    for (U32 i = 0; i < count; i++)
    {
        new (data + i) T();
    }

    finalizer->next = m_ArrayFinalizerChain;
    finalizer->destructor = &DestructorCall<T>;
    finalizer->objectsCount = count;
    finalizer->objectSize = sizeof(T);
    finalizer->objectsOffset = (U8)PtrDifference(data, finalizer);

    m_ArrayFinalizerChain = finalizer;

    return data;
}

template<typename T>
inline void* AllocationScope::AllocPod()
{
    return m_Allocator->Alloc(sizeof(T), alignof(T), m_Level);
}

template<typename T>
inline void* AllocationScope::AllocPodArray(U32 count)
{
    return m_Allocator->Alloc(sizeof(T) * count, alignof(T), m_Level);
}


DRE_END_NAMESPACE
