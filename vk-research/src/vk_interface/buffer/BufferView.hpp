#pragma once

#include <vulkan/vulkan.h>

#include "../resources/Resource.hpp"

namespace VKW
{

struct BufferViewHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct BufferView
{
    VkBufferView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    BufferResourceHandle resource_;
    std::uint64_t offset_ = 0;
    std::uint64_t size_ = 0;
};

}