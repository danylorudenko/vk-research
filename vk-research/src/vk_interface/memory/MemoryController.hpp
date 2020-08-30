#pragma once

#include <cstdint>
#include <vector>
#include <vulkan\vulkan.h>

#include <class_features\NonCopyable.hpp>
#include <vk_interface\memory\Memory.hpp>

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

    void ProvideMemoryRegion(MemoryPageRegionDesc const& desc, MemoryRegion& regionOut);
    void ReleaseMemoryRegion(MemoryRegion& region);

    MemoryPage* GetPage(MemoryPageHandle handle);

private:
    MemoryPageHandle AllocPage(MemoryAccessBits access, MemoryUsage usage, std::uint64_t size);
    void FreePage(MemoryPageHandle pageIndex);

    void GetNextFreePageRegion(MemoryPageHandle page, MemoryPageRegionDesc const& desc, MemoryRegion& regionOut);

    void AssignDefaultPageSizes();
    void ClassifyDeviceMemoryTypes();

private:
    ImportTable*    table_;
    Device*         device_;

    VkDeviceSize    defaultPageSizes_[(int)MemoryUsage::MAX];
    std::uint32_t   memoryClassTypes[(int)MemoryClass::MAX];

    std::vector<MemoryPage*> allocations_;
};

}