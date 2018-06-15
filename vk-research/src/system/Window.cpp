#include "Window.hpp"

WindowClass::WindowClass()
    : name_{}
    , registered_{ false }
{ 
}

WindowClass::WindowClass(HINSTANCE instance, char const* name, WinProcHandler handler)
    : name_{ name }
    , registered_{ false }
{
    WNDCLASS wc = {
        CS_VREDRAW | CS_HREDRAW,
        handler,
        0, 0,
        instance,
        LoadIcon(nullptr, IDI_APPLICATION),
        LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        name_.c_str()
    };

    registered_ = ::RegisterClass(&wc) != 0;
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

WindowClass::~WindowClass()
{
    if(registered_)
        ::UnregisterClass(name_.c_str(), ::GetModuleHandle(NULL));

    registered_ = false;
}

std::string const& WindowClass::Name() const
{
    return name_;
}

WindowClass::operator bool() const
{
    return registered_;
}


//////////////////////////////////////////
Window::Window()
    : windowClass_{}
    , handle_{ NULL }
    , title_{}
    , width_{ 0 }
    , height_{ 0 }
    , userData_{ nullptr }
{ 
}

Window::Window(HINSTANCE instance, char const* title, std::uint32_t width, std::uint32_t height, char const* className, WindowClass::WinProcHandler procHandler, void* userData)
    : windowClass_{ instance, className, procHandler }
    , handle_{ NULL }
    , title_{ title }
    , width_{ width }
    , height_{ height }
    , userData_{ userData }
{
    if (windowClass_) {
        handle_ = ::CreateWindow(windowClass_.Name().c_str(), title_.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width_, height_, nullptr, nullptr, instance, nullptr);

        if (handle_) {
            ::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

            ::ShowWindow(handle_, SW_SHOWDEFAULT);
            ::UpdateWindow(handle_);
        }
    }
}

Window::Window(Window&& rhs)
    : windowClass_{ std::move(rhs.windowClass_) }
    , handle_{ std::move(rhs.handle_) }
    , title_{ std::move(rhs.title_) }
    , width_{ std::move(rhs.width_) }
    , height_{ std::move(rhs.height_) }
    , userData_{ std::move(rhs.userData_) }
{
    ::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

Window& Window::operator=(Window&& rhs)
{
    windowClass_ = std::move(rhs.windowClass_);
    handle_ = std::move(rhs.handle_); rhs.handle_ = NULL;
    title_ = std::move(rhs.title_);
    width_ = std::move(rhs.width_); rhs.width_ = 0;
    height_ = std::move(rhs.height_); rhs.height_ = 0;
    userData_ = std::move(rhs.userData_); rhs.userData_ = nullptr;
    
    ::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

Window::~Window()
{
    if (handle_) {
        ::DestroyWindow(handle_);
    }

    handle_ = NULL;
}

Window::NativeWindowHandle Window::NativeHandle() const
{
    return handle_;
}

std::uint32_t Window::Width() const
{
    return width_;
}

std::uint32_t Window::Height() const
{
    return height_;
}
