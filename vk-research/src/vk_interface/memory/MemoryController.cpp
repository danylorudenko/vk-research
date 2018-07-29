#include "MemoryController.hpp"
#include "..\Device.hpp"
#include "..\Tools.hpp"
#include "..\resources\BufferLoader.hpp"

#include <algorithm>

namespace VKW
{

MemoryController::MemoryController()
    : table_{ nullptr }
    , device_{ nullptr }
{ 
}

MemoryController::MemoryController(MemoryControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    defaultPageSizes_[VERTEX_INDEX] = 1024 * 1024;
    defaultPageSizes_[UPLOAD_BUFFER] = 1024 * 1024 * 5;
    defaultPageSizes_[UNIFORM] = 1024 * 512;
    defaultPageSizes_[SAMPLE_TEXTURE] = 1024 * 1024 * 64;
    defaultPageSizes_[COLOR_ATTACHMENT] = 1024 * 1024 * 64;
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

void MemoryController::ProvideMemoryPageRegion(MemoryPageRegionDesc desc, MemoryPageRegion& regionOut)
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
            auto const accessValid = (page.accessFlags_ & accessFlags) == accessFlags;
            auto const sizeValid = desc.size_ >= (page.size_ - page.nextFreeOffset_);
            auto const usageValid = desc.usage_ == page.usage_;

            return accessValid && sizeValid && usageValid;
        });

    if (validPage != allocations_.end()) {
        GetNextFreePageRegion(*validPage, desc, regionOut);
    }
    else {
        auto const defaultPageSize = defaultPageSizes_[desc.usage_];
        auto const requestedSize = desc.size_ + desc.alignment_;
        auto const pageSize = requestedSize > defaultPageSize ? requestedSize : defaultPageSize;

        auto newPage = AllocPage(accessFlags, desc.usage_, pageSize);
        GetNextFreePageRegion(newPage, desc, regionOut);
    }
}

void MemoryController::GetNextFreePageRegion(MemoryPage& page, MemoryPageRegionDesc& desc, MemoryPageRegion& regionOut)
{
    auto const size = desc.size_ + desc.alignment_;

    regionOut.page_ = &page;
    regionOut.offset_ = RoundToMultipleOfPOT(page.nextFreeOffset_, desc.alignment_);
    regionOut.size_ = size;

    page.nextFreeOffset_ += size;
}

MemoryPage& MemoryController::AllocPage(MemoryAccess accessFlags, MemoryUsage usage, std::uint64_t size)
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
        if ((properties[i].propertyFlags & memoryFlags) == memoryFlags) {
            typeIndex = i;
            break;
        }
    }

    assert(typeIndex != VK_MAX_MEMORY_TYPES && "Could'nt find memory type for allocation.");

    VkMemoryAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.allocationSize = size;
    info.memoryTypeIndex = typeIndex;

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkAllocateMemory(device_->Handle(), &info, nullptr, &deviceMemory));

    MemoryPage memory;
    memory.deviceMemory_ = deviceMemory;
    memory.size_ = info.allocationSize;
    memory.memoryTypeId_ = typeIndex;
    memory.propertyFlags_ = properties[typeIndex].propertyFlags;
    memory.accessFlags_ = accessFlags;
    memory.usage_ = usage;
    
    memory.bindCount_ = 0;
    memory.nextFreeOffset_ = 0;

    allocations_.emplace_back(memory);

    return allocations_[allocations_.size() - 1];
}

void MemoryController::FreePage(std::uint64_t pageIndex)
{
    auto& page = allocations_[pageIndex];
    table_->vkFreeMemory(device_->Handle(), page.deviceMemory_, nullptr);
    allocations_.erase(allocations_.begin() + pageIndex);
}

}