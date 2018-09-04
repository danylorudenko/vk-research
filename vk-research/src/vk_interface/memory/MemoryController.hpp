#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "..\..\class_features\NonCopyable.hpp"
#include "Memory.hpp"

namespace VKW
{


struct MemoryPageRegionDesc
{
    std::uint64_t size_;
    std::uint64_t alignment_;
    std::uint32_t memoryTypeBits_;
    MemoryUsage usage_;
};



class Device;
class ImportTable;

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

    void GetNextFreePageRegion(MemoryPage& page, MemoryPageRegionDesc const& desc, MemoryRegion& regionOut);

private:
    ImportTable* table_;
    Device* device_;

    VkDeviceSize defaultPageSizes_[MemoryUsage::MAX];

    std::vector<MemoryPage> allocations_;
};

}