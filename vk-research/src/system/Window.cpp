#include "Window.hpp"

WindowClass::WindowClass(HINSTANCE instance, char const* name, WinProcHandler handler)
    : name_{ name }
    , registered_{ false }
{
    WNDCLASS classDesc;
    //classDesc.

}

WindowClass::WindowClass(WindowClass&& rhs)
{
    operator=(std::move(rhs));
}

WindowClass& WindowClass::operator=(WindowClass&& rhs)
{
    name_ = std::move(rhs.name_);
    registered_ = rhs.registered_;
    rhs.registered_ = false;

    return *this;
}

WindowClass::operator bool() const
{
    return registered_;
}
