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
    std::uint32_t memoryTypeBits_; // for asserts
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

    MemoryPageRegion AllocateMemoryRegion(MemoryPageRegionDesc const& desc);
    void ReleaseMemoryRegion(MemoryPageRegion& region);

private:
    MemoryPage* AllocPage(MemoryClass memoryClass, std::uint64_t size);
    void FreePage(MemoryPage* page);

    MemoryPageRegion GetNextFreePageRegion(MemoryPage* page, MemoryPageRegionDesc const& desc);

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