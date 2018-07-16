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



class MemoryController
    : public NonCopyable
{
public:
    MemoryController();
    MemoryController(VulkanImportTable* table, Device* device);

    MemoryController(MemoryController&& rhs);
    MemoryController& operator=(MemoryController&& rhs);

    ~MemoryController();

private:
    VkDeviceMemory AllocMemory(MemoryAccess access, std::uint64_t size);

private:
    VulkanImportTable* table_;
    Device* device_;

    std::vector<VkDeviceMemory> allocations_;
};

}