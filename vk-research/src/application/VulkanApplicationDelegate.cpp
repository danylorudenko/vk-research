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
    //auto& device = vulkanLoader_.Device();

    //VKW::BufferCreateInfo buffInfo;
    //buffInfo.size_ = 256;
    //buffInfo.usage_ = VKW::BufferUsage::VERTEX_INDEX;

    //VKW::Buffer buffer = vulkanLoader_.BufferLoader().LoadBuffer(buffInfo);

    ///////

    //vulkanLoader_.BufferLoader().UnloadBuffer(buffer);

    VKW::ImportTable const& table = vulkanLoader_.Table();
    VKW::Device& device = vulkanLoader_.Device();

    VkBuffer buffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;
    bufferInfo.size = 256;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.flags = VK_FLAGS_NONE;

    VK_ASSERT(table.vkCreateBuffer(device.Handle(), &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    table.vkGetBufferMemoryRequirements(device.Handle(), buffer, &memRequirements);

    std::uint32_t memType = VK_MAX_MEMORY_TYPES;
    VkFlags requiredMemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (auto i = 0u; i < device.Properties().memoryProperties.memoryTypeCount; ++i) {
        auto memProps = device.Properties().memoryProperties.memoryTypes[i].propertyFlags;
        if (((memProps & requiredMemoryProperty) == requiredMemoryProperty) && 
            (((1 << i) & memRequirements.memoryTypeBits) != 0)) {
            memType = i;
            break;
        }
    }
    assert(memType != VK_MAX_MEMORY_TYPES);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = 1024 * 1024;
    allocInfo.memoryTypeIndex = memType;

    VkDeviceMemory memory;
    VK_ASSERT(table.vkAllocateMemory(device.Handle(), &allocInfo, nullptr, &memory));

    VK_ASSERT(table.vkBindBufferMemory(device.Handle(), buffer, memory, 0));

}

void VulkanApplicationDelegate::update()
{

}

void VulkanApplicationDelegate::shutdown()
{

}