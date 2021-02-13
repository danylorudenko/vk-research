#pragma once

#include <foundation\Common.hpp>

DRE_BEGIN_NAMESPACE

inline void DreMemZero(void* mem, U64 size)
{
    ::ZeroMemory(mem, size);
}

inline void DreMemSet(void* mem, U64 size, U32 val)
{
    ::FillMemory(mem, size, val);
}

DRE_END_NAMESPACE

