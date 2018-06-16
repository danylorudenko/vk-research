#include "DynamicLibrary.hpp"

#include <cassert>
#include <utility>

DynamicLibrary::DynamicLibrary()
    : handle_{ NULL }
{
}

DynamicLibrary::DynamicLibrary(char const* name)
    : handle_{ NULL }
{
    handle_ = LoadLibraryA(name);
    assert(handle_ != NULL);
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& rhs)
    : handle_{ nullptr }
{
    operator=(std::move(rhs));
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& rhs)
{
    std::swap(handle_, rhs.handle_);
    return *this;
}

DynamicLibrary::operator bool() const
{
    return handle_ != nullptr;
}

DynamicLibrary::~DynamicLibrary()
{
    if(handle_ != NULL)
        assert(FreeLibrary(handle_) != 0);

    handle_ = NULL;
}