#pragma once

#include <vulkan\vulkan.h>

#include <limits>

#include <vk_interface\memory\Memory.hpp>

namespace VKW
{


struct BufferResource
{
    BufferResource(VkBuffer handle, std::uint32_t size, MemoryPageRegion const& memory);

    VkBuffer handle_ = VK_NULL_HANDLE;
    std::uint32_t size_ = 0;
    MemoryPageRegion memoryRegion_;
};


struct ImageResource
{
    ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryPageRegion const& memory);

    VkImage handle_ = VK_NULL_HANDLE;
    VkFormat format_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    MemoryPageRegion memoryRegion_;
};

}