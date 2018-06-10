#pragma once

#include <string>

#include <Windows.h>

class WindowClass
{
    using WinProcHandler = LRESULT (*)(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

public:
    WindowClass(HINSTANCE instance, char const* name, WinProcHandler handler);

    WindowClass(WindowClass&& rhs);
    WindowClass(WindowClass const& rhs) = delete;

    WindowClass& operator=(WindowClass&& rhs);
    WindowClass& operator=(WindowClass const& rhs) = delete;

    operator bool() const;

    ~WindowClass();

    
private:
    std::string name_;
    bool registered_;
    
};