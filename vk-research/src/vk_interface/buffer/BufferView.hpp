#pragma once

#include <vulkan/vulkan.h>

#include "ProvidedBuffer.hpp"

namespace VKW
{

struct BufferView
{
    BufferView(VkBufferView view, VkFormat format, std::uint64_t offset, std::uint64_t size, ProvidedBuffer* providedBuffer);

    VkBufferView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    std::uint64_t offset_ = 0;
    std::uint64_t size_ = 0;
    ProvidedBuffer* providedBuffer_ = nullptr; // for BuffersProvider's use
};

struct BufferViewHandle
{
    BufferView* view_;
};

}