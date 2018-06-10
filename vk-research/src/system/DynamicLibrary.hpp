#pragma once

#include <Windows.h>

class DynamicLibrary final
{
public:
    DynamicLibrary(char const* name);

    DynamicLibrary(DynamicLibrary const& rhs) = delete;
    DynamicLibrary(DynamicLibrary&& rhs);

    DynamicLibrary& operator=(DynamicLibrary const& rhs) = delete;
    DynamicLibrary& operator=(DynamicLibrary&& rhs);

    operator bool() const;

    ~DynamicLibrary();

private:
    HMODULE handle_;
};