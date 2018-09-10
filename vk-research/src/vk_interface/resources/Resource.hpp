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

struct BufferHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};



struct ImageResource
{
    VkImage handle_;
};

struct ImageHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}