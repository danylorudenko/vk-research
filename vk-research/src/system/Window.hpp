#pragma once

#include <string>
#include <cstddef>

#include <Windows.h>

class WindowClass
{
public:
    using WinProcHandler = LRESULT (*)(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

public:
    WindowClass(HINSTANCE instance, char const* name, WinProcHandler handler);

    WindowClass(WindowClass&& rhs);
    WindowClass(WindowClass const& rhs) = delete;

    WindowClass& operator=(WindowClass&& rhs);
    WindowClass& operator=(WindowClass const& rhs) = delete;

    operator bool() const;

    std::string const& Name() const;

    ~WindowClass();

    
private:
    std::string name_;
    bool registered_;
    
};



class Window
{
public:
    using NativeWindowHandle = HWND;

public:
    Window(HINSTANCE instance, char const* title, std::uint32_t width, std::uint32_t height, char const* className, WindowClass::WinProcHandler procHandler, void* userData);
    
    Window(Window&& rhs);
    Window(Window const& rhs) = delete;

    Window& operator=(Window&& rhs);
    Window& operator=(Window const& rhs) = delete;

    ~Window();

    NativeWindowHandle NativeHandle() const;

    std::uint32_t Width() const;
    std::uint32_t Height() const;

private:
    WindowClass windowClass_;
    NativeWindowHandle handle_;

    std::string title_;

    std::uint32_t width_;
    std::uint32_t height_;

    void* userData_;
};