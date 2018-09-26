#pragma once

#include <limits>
#include <cstdint>

namespace VKW
{

enum MemoryUsage
{
    VERTEX_INDEX = 0,
    UPLOAD_BUFFER,
    UNIFORM,
    SAMPLE_TEXTURE,
    COLOR_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT,

    BEGIN = VERTEX_INDEX,
    END = COLOR_ATTACHMENT + 1,
    MAX = END
};

enum MemoryAccess
{
    NONE = 0,
    GPU_LOCAL = 1,
    CPU_READBACK = 2,
    CPU_WRITE = 4,
    CPU_COHERENT = 8,
};

struct MemoryPageHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct MemoryPage
{
    VkDeviceMemory deviceMemory_;
    VkDeviceSize size_;
    std::uint32_t memoryTypeId_;
    VkMemoryPropertyFlags propertyFlags_;
    MemoryAccess accessFlags_;
    MemoryUsage usage_;

    std::uint32_t bindCount_;
    VkDeviceSize nextFreeOffset_;
};

struct MemoryRegion
{
    MemoryPageHandle pageHandle_;
    std::uint64_t offset_;
    std::uint64_t size_;
};

}