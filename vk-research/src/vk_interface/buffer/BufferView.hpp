#pragma once

#include <vulkan/vulkan.h>

#include "../resources/Resource.hpp"

namespace VKW
{

struct BuffersProvider::ProvidedBuffer;

struct BufferView
{
    BufferView(VkBufferView view, VkFormat format, BufferResourceHandle buffer, std::uint64_t offset, std::uint64_t size);

    VkBufferView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    std::uint64_t offset_ = 0;
    std::uint64_t size_ = 0;
    BufferResourceHandle resource_;
    BuffersProvider::ProvidedBuffer* providedBuffer_ = std::numeric_limits<std::uint32_t>::max(); // for BuffersProvider's use
};

struct BufferViewHandle
{
    BufferView* view_;
};

}