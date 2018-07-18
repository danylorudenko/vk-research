#pragma once

#include "../Device.hpp"

namespace VKW
{

enum MemoryAccess
{
    GPU_ONLY = 1,
    CPU_READBACK = 2,
    CPU_WRITE = 4,
    CPU_COHERENT = 8,
};


struct MemoryPage
{
    VkDeviceMemory deviceMemory_;
    VkDeviceSize size_;
    VkMemoryPropertyFlags propertyFlags_;
    MemoryAccess accessFlags_;

    std::uint32_t bindCount_;
    VkDeviceSize nextFreeOffset_;
};

struct MemoryPageRegion
{
    MemoryPage* page_;
    std::size_t offset_;
    std::size_t size_;
};



class MemoryController
    : public NonCopyable
{
public:
    MemoryController();
    MemoryController(VulkanImportTable* table, Device* device);

    MemoryController(MemoryController&& rhs);
    MemoryController& operator=(MemoryController&& rhs);

    ~MemoryController();

public:
    void ProvideMemoryPageRegion(std::size_t size, std::size_t alignment, MemoryAccess access, MemoryPageRegion& region);

private:
    MemoryPage& AllocPage(MemoryAccess access, std::uint64_t size);
    void FreePage(std::size_t pageIndex);

private:
    VulkanImportTable* table_;
    Device* device_;

    std::vector<MemoryPage> allocations_;
};

}