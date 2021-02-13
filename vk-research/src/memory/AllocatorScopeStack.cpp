#include <memory\AllocatorScopeStack.hpp>


DRE_BEGIN_NAMESPACE

AllocationScope::~AllocationScope()
{
    ArrayFinalizer* arrayFinalizer = m_ArrayFinalizerChain;
    while (arrayFinalizer != nullptr)
    {
        void* object = PtrAdd(arrayFinalizer, (PtrDiff)arrayFinalizer->objectsOffset);
        auto const destructor = arrayFinalizer->destructor;
        U32 const objSize = arrayFinalizer->objectSize;
        U32 const count = arrayFinalizer->objectsCount;
        for (U32 i = 0; i < count; i++)
        {
            destructor(object);
            object = PtrAdd(object, objSize);
        }

        arrayFinalizer = arrayFinalizer->next;
    }
    
    Finalizer* finalizer = m_FinalizerChain;
    while (finalizer != nullptr)
    {
        void* object = PtrAdd(finalizer, (PtrDiff)finalizer->objectOffset);
        finalizer->destructor(object);
        finalizer = finalizer->next;
    }
    
    m_Allocator->Unwind(m_UnwindPoint, m_Level);
}

void* AllocationScope::Alloc(U64 size, U32 alignment)
{
    return m_Allocator->Alloc(size, alignment, m_Level);
}

DRE_END_NAMESPACE

