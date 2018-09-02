#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "..\..\class_features\NonCopyable.hpp"
#include "..\ImportTable.hpp"

namespace VKW
{


enum MemoryUsage
{
    VERTEX_INDEX = 0,
    UPLOAD_BUFFER,
    UNIFORM,
    SAMPLE_TEXTURE,
    COLOR_ATTACHMENT,

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
    MemoryPage* page_;
    std::uint64_t offset_;
    std::uint64_t size_;
};

struct MemoryPageRegionDesc
{
    std::uint64_t size_;
    std::uint64_t alignment_;
    std::uint32_t memoryTypeBits_;
    MemoryUsage usage_;
};



class Device;

struct MemoryControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class MemoryController
    : public NonCopyable
{
public:
    MemoryController();
    MemoryController(MemoryControllerDesc const& desc);

    MemoryController(MemoryController&& rhs);
    MemoryController& operator=(MemoryController&& rhs);

    ~MemoryController();

public:
    void ProvideMemoryRegion(MemoryPageRegionDesc const& desc, MemoryRegion& regionOut);
    void ReleaseMemoryRegion(MemoryRegion& region);

private:
    MemoryPage& AllocPage(MemoryAccess access, MemoryUsage usage, std::uint64_t size);
    void FreePage(std::uint64_t pageIndex);

    void GetNextFreePageRegion(MemoryPage& page, MemoryPageRegionDesc& desc, MemoryRegion& regionOut);

private:
    ImportTable* table_;
    Device* device_;

    VkDeviceSize defaultPageSizes_[MemoryUsage::MAX];

    std::vector<MemoryPage> allocations_;
};

}