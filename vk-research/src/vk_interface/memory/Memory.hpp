#pragma once

#include <limits>

#include <vk_interface\Tools.hpp>
#include <vulkan/vulkan.h>

namespace VKW
{

enum class MemoryClass
{
    DeviceFast,
    CpuUniform,
    CpuStaging,
    CpuReadback,
    MAX
};

struct MemoryPage
{
    VkDeviceMemory deviceMemory_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
    MemoryClass memoryClass;
    void* mappedMemoryPtr_;

    std::uint32_t bindCount_ = 0;
    VkDeviceSize nextFreeOffset_ = 0;

    VkDeviceSize GetFreeMemorySize() const { return size_ - nextFreeOffset_; }
};

struct MemoryPageRegion
{
    MemoryPage*     page_;
    std::uint64_t   offset_ = 0;
    std::uint64_t   size_ = 0;
};

}