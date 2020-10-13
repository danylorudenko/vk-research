#pragma once

#include <vulkan\vulkan.h>
#include <cstdint>

#include <vk_interface/resources/Resource.hpp>

namespace VKW
{

struct BufferView
{
    BufferView(VkBufferView view, VkFormat format, std::uint64_t offset, std::uint64_t size, BufferResource* bufferResource, std::uint32_t* counter);

    VkBufferView    handle_ = VK_NULL_HANDLE;
    VkFormat        format_ = VK_FORMAT_UNDEFINED;
    std::uint64_t   offset_ = 0;
    std::uint64_t   size_ = 0;
    BufferResource* bufferResource_ = nullptr;
    std::uint32_t*  bufferResourceReferenceCount_ = nullptr;
};

}