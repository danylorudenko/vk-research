#pragma once

#include <vulkan/vulkan.h>

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
};

struct SubbufferResource
{
    VkBuffer handle_;
    std::uint32_t offset_;
    std::uint32_t size_;
};

struct ImageResource
{
    VkImage handle_;
};

}