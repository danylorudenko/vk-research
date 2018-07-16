#include "MemoryController.hpp"
#include "../Tools.hpp"

namespace VKW
{

MemoryController::MemoryController()
    : table_{ nullptr }
    , device_{ nullptr }
{ 
}

MemoryController::MemoryController(VulkanImportTable* table, Device* device)
    : table_{ table }
    , device_{ device }
{

}

MemoryController::MemoryController(MemoryController&& rhs)
{
    operator=(std::move(rhs));
}

MemoryController& MemoryController::operator=(MemoryController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(allocations_, rhs.allocations_);

    return *this;
}

MemoryController::~MemoryController()
{
}

VkDeviceMemory MemoryController::AllocMemory(MemoryAccess access, std::uint64_t size)
{
    VkDeviceMemory memory = VK_NULL_HANDLE;

    VkMemoryPropertyFlags memoryFlags = VK_FLAGS_NONE;
    switch (access) {
    case MemoryAccess::GPU_ONLY:
        memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case MemoryAccess::CPU_WRITE:
        memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        break;
    case MemoryAccess::CPU_READBACK:
        assert(false && "MemoryAccess::CPU_READBACK not implemented");
        break;
    }

    //VK_ASSERT(table_->vkAllocateMemory(device_->Handle(), &info, nullptr, &memory));
    return memory;
}

}