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
    MemoryClass memoryClass_;
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
    MemoryPageHandle AllocPage(MemoryClass memoryClass, std::uint64_t size);
    void FreePage(MemoryPageHandle pageIndex);

    void GetNextFreePageRegion(MemoryPageHandle page, MemoryPageRegionDesc const& desc, MemoryRegion& regionOut);

    void AssignDefaultProperties();
    void ClassifyDeviceMemoryTypesAll();
    std::uint32_t FindBestMemoryType(std::uint32_t mandatoryFlags, std::uint32_t preferebleFlags, std::uint32_t nonPreferableFlags);

private:
    ImportTable*    table_;
    Device*         device_;

    std::uint32_t   memoryClassTypes_[(int)MemoryClass::MAX];
    VkDeviceSize    defaultPageSizes_[(int)MemoryClass::MAX];

    std::vector<MemoryPage*> allocations_;
};

}