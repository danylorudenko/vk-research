#pragma once

#include <string>
#include <cstddef>

#include <Windows.h>

#include "..\class_features\NonCopyable.hpp"

class WindowClass
    : public NonCopyable
{
public:
    using WinProcHandler = LRESULT (*)(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

public:
    WindowClass();
    WindowClass(HINSTANCE instance, char const* name, WinProcHandler handler);

    WindowClass(WindowClass&& rhs);
    WindowClass& operator=(WindowClass&& rhs);

    operator bool() const;

    std::string const& Name() const;

    ~WindowClass();

    
private:
    std::string name_;
    bool registered_;
    
};



class Window
    : public NonCopyable
{
public:
    using NativeWindowHandle = HWND;

public:
    Window();
    Window(HINSTANCE instance, char const* title, std::uint32_t width, std::uint32_t height, char const* className, WindowClass::WinProcHandler procHandler, void* userData);
    
    Window(Window&& rhs);

    Window& operator=(Window&& rhs);

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