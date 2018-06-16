#pragma once

#include <Windows.h>
#include <cassert>

#include "..\class_features\NonCopyable.hpp"

class DynamicLibrary final
    : public NonCopyable
{
public:
    DynamicLibrary();
    DynamicLibrary(char const* name);

    DynamicLibrary(DynamicLibrary&& rhs);
    DynamicLibrary& operator=(DynamicLibrary&& rhs);

    operator bool() const;

    template<typename TProc>
    TProc* GetProcAddress(char const* procName)
    {
        assert(handle_ != NULL);
        return reinterpret_cast<TProc*>(::GetProcAddress(handle_, procName));
    }

    ~DynamicLibrary();

private:
    HMODULE handle_;
};