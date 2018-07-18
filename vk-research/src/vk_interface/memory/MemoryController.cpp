#include "MemoryController.hpp"
#include "../Tools.hpp"

#include <algorithm>

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
    for (auto const& memory : allocations_) {
        table_->vkFreeMemory(device_->Handle(), memory.deviceMemory_, nullptr);
    }
}

void MemoryController::ProvideMemoryPageRegion(MemoryPageRegionDesc desc, MemoryPageRegion& region)
{
    MemoryAccess accessFlags = MemoryAccess::NONE;

    switch (desc.usage_)
    {
    case MemoryUsage::VERTEX_INDEX:
        accessFlags = BitwiseEnumOR32(MemoryAccess::GPU_LOCAL, accessFlags);
        break;
    case MemoryUsage::UNIFORM:
        accessFlags = BitwiseEnumOR32(MemoryAccess::CPU_WRITE, accessFlags);
        break;
    case MemoryUsage::SAMPLE_TEXTURE:
        accessFlags = BitwiseEnumOR32(MemoryAccess::GPU_LOCAL, accessFlags);
        break;
    case MemoryUsage::UPLOAD_BUFFER:
        accessFlags = BitwiseEnumOR32(MemoryAccess::CPU_WRITE, accessFlags);
        break;
    }

    auto validPage = std::find_if(allocations_.begin(), allocations_.end(),
        [&desc, accessFlags](MemoryPage const& page)
        {
            auto const usageValid = (page.accessFlags_ & accessFlags) == accessFlags;
            auto const sizeValid = desc.size_ >= (page.size_ - page.nextFreeOffset_);

            return usageValid && sizeValid;
        });

    if (validPage != allocations_.end()) {
        GetNextFreePageRegion(*validPage, desc, region);
    }
    else {
        auto const pageSize = defaultPageSizes_[desc.usage_];
        auto newPage = AllocPage(accessFlags, desc.usage_, pageSize);
        GetNextFreePageRegion(newPage, desc, region);
    }
}

void MemoryController::GetNextFreePageRegion(MemoryPage& page, MemoryPageRegionDesc& desc, MemoryPageRegion& region)
{
    auto const alignedSize = RoundToMultipleOfPOT(desc.size_, desc.alignment_);

    region.page_ = &page;
    region.offset_ = page.nextFreeOffset_;
    region.size_ = alignedSize;

    page.nextFreeOffset_ += alignedSize;
}

MemoryPage& MemoryController::AllocPage(MemoryAccess accessFlags, MemoryUsage usage, std::size_t size)
{    
    VkMemoryPropertyFlags memoryFlags = VK_FLAGS_NONE;

    if (accessFlags & MemoryAccess::CPU_COHERENT)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (accessFlags & MemoryAccess::GPU_LOCAL)
        memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (accessFlags & MemoryAccess::CPU_WRITE)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if (accessFlags & MemoryAccess::CPU_READBACK)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    auto const propertiesCount = device_->Properties().memoryProperties.memoryTypeCount;
    auto const* properties = device_->Properties().memoryProperties.memoryTypes;

    std::uint32_t typeIndex = VK_MAX_MEMORY_TYPES;
    for (auto i = 0u; i < propertiesCount; ++i) {
        // first fit
        if((properties[i].propertyFlags & memoryFlags) == memoryFlags)
            typeIndex = properties[i].heapIndex;
    }

    assert(typeIndex != VK_MAX_MEMORY_TYPES && "Could'nt find memory type for allocation.");

    VkMemoryAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.allocationSize = RoundToMultipleOfPOT(size, 256);
    info.memoryTypeIndex = typeIndex;

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkAllocateMemory(device_->Handle(), &info, nullptr, &deviceMemory));

    MemoryPage memory;
    memory.deviceMemory_ = deviceMemory;
    memory.size_ = info.allocationSize;
    memory.propertyFlags_ = properties[typeIndex].propertyFlags;
    memory.accessFlags_ = accessFlags;
    memory.usage_ = usage;
    
    memory.bindCount_ = 0;
    memory.nextFreeOffset_ = 0;

    allocations_.emplace_back(memory);

    return allocations_[allocations_.size() - 1];
}

void MemoryController::FreePage(std::size_t pageIndex)
{
    auto& page = allocations_[pageIndex];
    table_->vkFreeMemory(device_->Handle(), page.deviceMemory_, nullptr);
    allocations_.erase(allocations_.begin() + pageIndex);
}

}