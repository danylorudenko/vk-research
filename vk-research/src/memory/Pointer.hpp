#pragma once

#include <foundation\common.hpp>

DRE_BEGIN_NAMESPACE

inline PtrDiff PtrDifference(void* lhs, void* rhs)
{
    return (U8*)lhs - (U8*)rhs;
}

template<typename T>
inline T* PtrAdd(T* lhs, PtrDiff rhs)
{
    return (T*)((U8*)lhs + rhs);
}

template<typename T>
inline T* PtrAlign(T* ptr, U32 alignment)
{
    return (T*)(((U64)ptr + alignment - 1) & (~((U64)alignment - 1)));
}

DRE_END_NAMESPACE

