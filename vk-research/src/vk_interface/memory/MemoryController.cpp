#include "MemoryController.hpp"
#include "..\Device.hpp"
#include "..\ImportTable.hpp"
#include "..\Tools.hpp"

#include <algorithm>

namespace VKW
{

MemoryController::MemoryController()
    : table_{ nullptr }
    , device_{ nullptr }
{
    AssignDefaultPageSizes();
}

MemoryController::MemoryController(MemoryControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    AssignDefaultPageSizes();
}

MemoryController::MemoryController(MemoryController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    AssignDefaultPageSizes();
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
        table_->vkFreeMemory(device_->Handle(), memory->deviceMemory_, nullptr);
        delete memory;
    }
}

void MemoryController::AssignDefaultPageSizes()
{
    defaultPageSizes_[VERTEX_INDEX] = 1024 * 1024;
    defaultPageSizes_[UPLOAD_BUFFER] = 1024 * 1024 * 5;
    defaultPageSizes_[UNIFORM] = 1024 * 512;
    defaultPageSizes_[SAMPLE_TEXTURE] = 1024 * 1024 * 64;
    defaultPageSizes_[DEPTH_STENCIL_ATTACHMENT] = 1024 * 1024 * 16;
    defaultPageSizes_[COLOR_ATTACHMENT] = 1024 * 1024 * 64;
}

MemoryPage* MemoryController::GetPage(MemoryPageHandle handle)
{
    return handle.page_;
}

void MemoryController::ProvideMemoryRegion(MemoryPageRegionDesc const& desc, MemoryRegion& regionOut)
{
    MemoryAccessBits accessFlags = MemoryAccessBits::NONE;

    switch (desc.usage_)
    {
    case MemoryUsage::VERTEX_INDEX:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::GPU_LOCAL, accessFlags);
        break;
    case MemoryUsage::UPLOAD_BUFFER:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::CPU_WRITE, accessFlags);
        break;
    case MemoryUsage::UNIFORM:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::CPU_WRITE, accessFlags);
        break;
    case MemoryUsage::SAMPLE_TEXTURE:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::GPU_LOCAL, accessFlags);
        break;
    case MemoryUsage::DEPTH_STENCIL_ATTACHMENT:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::GPU_LOCAL, accessFlags);
        break;
    case MemoryUsage::COLOR_ATTACHMENT:
        accessFlags = BitwiseEnumOR32(MemoryAccessBits::GPU_LOCAL, accessFlags);
        break;
    default:
        assert(false && "Unsupported MemoryUsage");
    }

    std::uint32_t constexpr INVALID_ALLOCATION = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t validAllocation = INVALID_ALLOCATION;
    auto const allocationsCount = allocations_.size();
    for (auto i = 0u; i < allocationsCount; ++i) {
        auto const page = allocations_[i];
        auto const accessValid = (page->accessFlags_ & accessFlags) == accessFlags;
        auto const sizeValid = desc.size_ <= (page->size_ - page->nextFreeOffset_);
        auto const usageValid = desc.usage_ == page->usage_;

        if (accessValid && sizeValid && usageValid) {
            validAllocation = i;
            break;
        }
    }

    if (validAllocation != INVALID_ALLOCATION) {
        GetNextFreePageRegion(MemoryPageHandle{ allocations_[validAllocation] }, desc, regionOut);
    }
    else {
        auto const defaultPageSize = defaultPageSizes_[desc.usage_];
        auto const requestedSize = desc.size_ + desc.alignment_;
        auto const pageSize = requestedSize > defaultPageSize ? requestedSize : defaultPageSize;

        MemoryPageHandle newPage = AllocPage(accessFlags, desc.usage_, pageSize);
        GetNextFreePageRegion(newPage, desc, regionOut);
    }
}

void MemoryController::GetNextFreePageRegion(MemoryPageHandle pageHandle, MemoryPageRegionDesc const& desc, MemoryRegion& regionOut)
{
    auto const size = desc.size_ + desc.alignment_;

    auto& page = *pageHandle.page_;
    regionOut.pageHandle_ = pageHandle;
    regionOut.offset_ = RoundToMultipleOfPOT(page.nextFreeOffset_, desc.alignment_);
    regionOut.size_ = size;

    page.nextFreeOffset_ += size;
    ++page.bindCount_;
}

void MemoryController::ReleaseMemoryRegion(MemoryRegion& region)
{
    auto const regionMemoryAllocation = region.pageHandle_.page_->deviceMemory_;

    std::uint32_t INVALID_PAGE = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t pageIndex = INVALID_PAGE;
    for (auto i = 0u; i < allocations_.size(); ++i) {
        if (regionMemoryAllocation == allocations_[i]->deviceMemory_) {
            pageIndex = i;
        }
    }

    if (pageIndex != INVALID_PAGE) {
        if (--allocations_[pageIndex]->bindCount_ == 0) {
            FreePage(MemoryPageHandle{ allocations_[pageIndex] });
        }
    }

    region.pageHandle_ = {};
    region.size_ = 0;
    region.offset_ = 0;
}

MemoryPageHandle MemoryController::AllocPage(MemoryAccessBits accessFlags, MemoryUsage usage, std::uint64_t size)
{    
    VkMemoryPropertyFlags memoryFlags = VK_FLAGS_NONE;

    if (accessFlags & MemoryAccessBits::CPU_COHERENT)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (accessFlags & MemoryAccessBits::GPU_LOCAL)
        memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (accessFlags & MemoryAccessBits::CPU_WRITE)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if (accessFlags & MemoryAccessBits::CPU_READBACK)
        memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    auto const memoryTypesCount = device_->Properties().memoryProperties.memoryTypeCount;
    auto const* memoryTypes = device_->Properties().memoryProperties.memoryTypes;

    std::uint32_t typeIndex = VK_MAX_MEMORY_TYPES;
    for (auto i = 0u; i < memoryTypesCount; ++i) {
        // first fit
        if ((memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags) {
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

    MemoryPage* memory = new MemoryPage{};
    memory->deviceMemory_ = deviceMemory;
    memory->size_ = info.allocationSize;
    memory->memoryTypeId_ = typeIndex;
    memory->propertyFlags_ = memoryTypes[typeIndex].propertyFlags;
    memory->accessFlags_ = accessFlags;
    memory->usage_ = usage;
    
    memory->bindCount_ = 0;
    memory->nextFreeOffset_ = 0;

    allocations_.emplace_back(memory);

    return MemoryPageHandle{ memory };
}

void MemoryController::FreePage(MemoryPageHandle pageHandle)
{
    std::uint32_t deletedPageIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t const allocationsCount = static_cast<std::uint32_t>(allocations_.size());
    for (auto i = 0u; i < allocationsCount; ++i) {
        if (pageHandle.page_ == allocations_[i]) {
            deletedPageIndex = i;
            break;
        }
    }
    
    auto* page = allocations_[deletedPageIndex];
    table_->vkFreeMemory(device_->Handle(), page->deviceMemory_, nullptr);

    delete page;
    allocations_.erase(allocations_.begin() + deletedPageIndex);
}

}