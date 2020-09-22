#pragma once

#include <limits>
#include <vk_interface\Tools.hpp>

namespace VKW
{

enum class MemoryClass
{
    DeviceFastMemory,
    CpuUniformMemory,
    CpuStagingMemory,
    CpuReadbackMemory,
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

struct MemoryPageHandle
{
    MemoryPage* page_;
};


struct MemoryRegion
{
    MemoryPageHandle pageHandle_;
    std::uint64_t offset_ = 0;
    std::uint64_t size_ = 0;
};

}