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

    if (access & MemoryAccess::CPU_COHERENT)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (access & MemoryAccess::GPU_ONLY)
        memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (access &MemoryAccess::CPU_WRITE)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if (access& MemoryAccess::CPU_READBACK)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    auto const propertiesCount = device_->Properties().memoryProperties.memoryTypeCount;
    auto const* properties = device_->Properties().memoryProperties.memoryTypes;

    asdqrb
    // INVALID DEFAULT VALUE
    std::uint32_t heapIndex = 0u;
    for (auto i = 0u; i < propertiesCount; ++i) {
        // first fit
        if((properties[i].propertyFlags & memoryFlags) == memoryFlags)
            heapIndex = properties[i].heapIndex;
    }

    //VK_ASSERT(table_->vkAllocateMemory(device_->Handle(), &info, nullptr, &memory));
    return memory;
}

}