#pragma once

#include <limits>
#include <cstdint>
#include <vulkan\vulkan.h>
#include "..\Tools.hpp"

namespace VKW
{

enum MemoryUsage
{
    VERTEX_INDEX = 0,
    UPLOAD_BUFFER,
    UNIFORM,
    SAMPLE_TEXTURE,
    STORAGE,
    DEPTH_STENCIL_ATTACHMENT,
    COLOR_ATTACHMENT,

    BEGIN = VERTEX_INDEX,
    END = COLOR_ATTACHMENT + 1,
    MAX = END
};

enum MemoryAccessBits
{
    NONE = 0,
    GPU_LOCAL = 1,
    CPU_READBACK = 1 << 1,
    CPU_WRITE = 1 << 2,
    CPU_COHERENT = 1 << 3,
};

struct MemoryPage
{
    VkDeviceMemory deviceMemory_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
    std::uint32_t memoryTypeId_ = std::numeric_limits<std::uint32_t>::max();
    VkMemoryPropertyFlags propertyFlags_ = VK_FLAGS_NONE;
    MemoryAccessBits accessFlags_;
    MemoryUsage usage_;
    void* mappedMemoryPtr_;

    std::uint32_t bindCount_ = 0;
    VkDeviceSize nextFreeOffset_ = 0;
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