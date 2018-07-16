#pragma once

#include "../Device.hpp"

namespace VKW
{

enum class MemoryAccess
{
    GPU_ONLY,
    CPU_READBACK,
    CPU_WRITE
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