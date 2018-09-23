#pragma once

#include <vulkan/vulkan.h>

#include <limits>

#include "../memory/Memory.hpp"

namespace VKW
{

enum class ResourceType
{
    Buffer,
    Image
};



struct BufferResource
{
    BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory);

    VkBuffer handle_;
    std::uint32_t size_;
    MemoryRegion memory_;
};

struct SubbufferResource
{
    VkBuffer handle_;
    std::uint32_t offset_;
    std::uint32_t size_;
};

struct BufferResourceHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};



struct ImageResource
{
    VkImage handle_;
    VkFormat format_;
};

struct ImageResourceHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}