#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\resources\BufferLoader.hpp"
#include "..\vk_interface\Tools.hpp"

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

    VKW::BufferCreateInfo buffInfo;
    buffInfo.size_ = 256;
    buffInfo.usage_ = VKW::BufferUsage::VERTEX_INDEX;

    VKW::Buffer buffer = vulkanLoader_.BufferLoader().LoadBuffer(buffInfo);
    VKW::Buffer buffer2 = vulkanLoader_.BufferLoader().LoadBuffer(buffInfo);

    ///////

    auto* worker = vulkanLoader_.WorkersSystem().GetWorker(VKW::WorkerType::GRAPHICS, 0);
    auto& frame = worker->StartNextExecutionFrame();


    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = 256;

    vulkanLoader_.Table().vkCmdCopyBuffer(frame.CommandBuffer(), buffer.handle_, buffer2.handle_, 1, &copyRegion);

    worker->ExecuteFrame(frame);

    frame.WaitForFence();


    ///////

    vulkanLoader_.BufferLoader().UnloadBuffer(buffer);

}

void VulkanApplicationDelegate::update()
{

}

void VulkanApplicationDelegate::shutdown()
{

}