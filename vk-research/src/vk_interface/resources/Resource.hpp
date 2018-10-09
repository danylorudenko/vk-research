#pragma once

#include <vulkan/vulkan.h>

#include <limits>

#include "../memory/Memory.hpp"

namespace VKW
{

enum class ResourceType
{
    Buffer,
    Image,
    Framebuffer
};



struct BufferResource
{
    BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory);

    VkBuffer handle_ = VK_NULL_HANDLE;
    std::uint32_t size_ = 0;
    MemoryRegion memory_;
};

struct SubbufferResource
{
    VkBuffer handle_ = VK_NULL_HANDLE;
    std::uint32_t offset_ = 0;
    std::uint32_t size_ = 0;
};

struct BufferResourceHandle
{
    BufferResource* resource_ = nullptr;
};



struct ImageResource
{
    ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryRegion const& memory);

    VkImage handle_ = VK_NULL_HANDLE;
    VkFormat format_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    MemoryRegion memory_;
};

struct ImageResourceHandle
{
    ImageResource* resource_ = nullptr;
};

}