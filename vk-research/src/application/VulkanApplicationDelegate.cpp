#include "VulkanApplicationDelegate.hpp"

VulkanApplicationDelegate::VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, bool vkDebug)
    : mainWindow_ {
        instance,
        title,
        windowWidth,
        windowHeight,
        "VulkanRenderWindow",
        VulkanApplicationDelegate::WinProc,
        this }
    , vulkanLoader_{ vkDebug }
{
}

VulkanApplicationDelegate::~VulkanApplicationDelegate()
{

}

LRESULT VulkanApplicationDelegate::WinProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    auto* appDelegate = reinterpret_cast<VulkanApplicationDelegate*>(::GetWindowLongPtr(handle, GWLP_USERDATA));
    
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    
    return ::DefWindowProc(handle, message, wparam, lparam);
}

void VulkanApplicationDelegate::start()
{
    auto& device = vulkanLoader_.Device();

    BufferCreateInfo buffInfo;
    buffInfo.size_ = 256;
    buffInfo.alignment_ = 4;
    buffInfo.usage_ = BufferUsage::VERTEX_INDEX;

    Buffer buffer = device.BufferLoader().LoadBuffer(buffInfo);

    ///////

    device.BufferLoader().UnloadBuffer(buffer);
}

void VulkanApplicationDelegate::update()
{

}

void VulkanApplicationDelegate::shutdown()
{

}