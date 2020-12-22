#include "MemoryController.hpp"

#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Tools.hpp>

#include <algorithm>
#include <limits>

namespace VKW
{

MemoryController::MemoryController()
    : table_{ nullptr }
    , device_{ nullptr }
{
    AssignDefaultProperties();
}

MemoryController::MemoryController(MemoryControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    //ClassifyDeviceMemoryTypesAll()
    AssignDefaultProperties();
}

MemoryController::MemoryController(MemoryController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

MemoryController& MemoryController::operator=(MemoryController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    ToolCopyMemoryArray(rhs.memoryClassTypes_, memoryClassTypes_);
    ToolCopyMemoryArray(rhs.defaultPageSizes_, defaultPageSizes_);
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

void MemoryController::AssignDefaultProperties()
{
    defaultPageSizes_[(int)MemoryClass::DeviceFast]       = 1024 * 1024 * 64;
    defaultPageSizes_[(int)MemoryClass::CpuStaging]       = 1024 * 1024 * 32;
    defaultPageSizes_[(int)MemoryClass::CpuUniform]       = 1024 * 1024 * 32;
    defaultPageSizes_[(int)MemoryClass::CpuReadback]      = 1024 * 1024 * 32;

    memoryClassTypes_[(int)MemoryClass::DeviceFast]       = TOOL_INVALID_ID;
    memoryClassTypes_[(int)MemoryClass::CpuStaging]       = TOOL_INVALID_ID;
    memoryClassTypes_[(int)MemoryClass::CpuUniform]       = TOOL_INVALID_ID;
    memoryClassTypes_[(int)MemoryClass::CpuReadback]      = TOOL_INVALID_ID;
}

VkDeviceSize GetMemoryTypeBudget(Device const& device ,std::uint32_t memoryType)
{
    VkPhysicalDeviceMemoryProperties2 const& memoryProperties2 = device.Properties().memoryProperties2;
    VkPhysicalDeviceMemoryProperties const& memoryProperties = memoryProperties2.memoryProperties;
    VkPhysicalDeviceMemoryBudgetPropertiesEXT const* budgetProperties = (VkPhysicalDeviceMemoryBudgetPropertiesEXT*)memoryProperties2.pNext;

    std::uint32_t const heapIndex = memoryProperties.memoryTypes[memoryType].heapIndex;

    VkDeviceSize const typeHeapSize = device.IsAPI11Supported()
        ? budgetProperties->heapBudget[heapIndex] - budgetProperties->heapUsage[heapIndex]
        : memoryProperties.memoryHeaps[heapIndex].size;

    return typeHeapSize;
}

std::uint32_t MemoryController::FindBestMemoryType(std::uint32_t mandatoryFlags, std::uint32_t preferableFlags, std::uint32_t nonPreferableFlags)
{
    std::uint32_t const memoryTypesCount = device_->Properties().memoryProperties2.memoryProperties.memoryTypeCount;

    std::uint32_t memoryTypeIndex = TOOL_INVALID_ID;
    std::int32_t prevRating = -1;
    for (std::uint32_t i = 0; i < memoryTypesCount; i++)
    {
        VkMemoryType const& type = device_->Properties().memoryProperties2.memoryProperties.memoryTypes[i];
        
        // not suitable at all
        if((type.propertyFlags & mandatoryFlags) != mandatoryFlags)
            continue;

        std::int32_t currentRating = static_cast<std::int32_t>(ToolCountBitsSet(preferableFlags & type.propertyFlags) 
            - ToolCountBitsSet(nonPreferableFlags & type.propertyFlags));
        if (currentRating > prevRating)
        {
            prevRating = currentRating;
            memoryTypeIndex = i;
        }
    }

    return memoryTypeIndex;
}

MemoryRegion MemoryController::AllocateMemoryRegion(MemoryPageRegionDesc const& desc)
{
    std::uint32_t constexpr INVALID_ALLOCATION = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t validAllocation = INVALID_ALLOCATION;
    auto const allocationsCount = allocations_.size();
    for (auto i = 0u; i < allocationsCount; ++i) {
        MemoryPage const* page = allocations_[i];
        bool const classValid  = page->memoryClass == desc.memoryClass_;
        bool const sizeValid   = desc.size_ <= page->GetFreeMemorySize();

        if (classValid && sizeValid ) {
            validAllocation = i;
            break;
        }
    }

    if (validAllocation != INVALID_ALLOCATION) {
        return GetNextFreePageRegion(allocations_[validAllocation], desc);
    }
    else {
        std::uint64_t const defaultPageSize = defaultPageSizes_[(int)desc.memoryClass_];
        std::uint64_t const requestedSize = desc.size_ + desc.alignment_;
        std::uint64_t const pageSize = requestedSize > defaultPageSize ? requestedSize : defaultPageSize;

        MemoryPage* newPage = AllocPage(desc.memoryClass_, pageSize);
        return GetNextFreePageRegion(newPage, desc);
    }
}

MemoryRegion MemoryController::GetNextFreePageRegion(MemoryPage* page, MemoryPageRegionDesc const& desc)
{
    std::uint64_t const size = desc.size_ + desc.alignment_;

    std::uint32_t const memoryTypeId = memoryClassTypes_[(int)page->memoryClass];
    assert((desc.memoryTypeBits_ & (1 << memoryTypeId)) && "Memory class of this MemoryPage has not fulfilled the allocation requirements."); 

    MemoryRegion result{ page, RoundToMultipleOfPOT(page->nextFreeOffset_, desc.alignment_), size };

    page->nextFreeOffset_ += size;
    ++page->bindCount_;
    
    return result;
}

void MemoryController::ReleaseMemoryRegion(MemoryRegion& region)
{
    auto const regionMemoryAllocation = region.page_->deviceMemory_;

    std::uint32_t constexpr INVALID_PAGE = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t pageIndex = INVALID_PAGE;
    for (auto i = 0u; i < allocations_.size(); ++i) {
        if (regionMemoryAllocation == allocations_[i]->deviceMemory_) {
            pageIndex = i;
        }
    }

    if (pageIndex != INVALID_PAGE) {
        if (--allocations_[pageIndex]->bindCount_ == 0) {
            FreePage(allocations_[pageIndex]);
        }
    }

    region.page_ = nullptr;
    region.size_ = 0;
    region.offset_ = 0;
}

MemoryPage* MemoryController::AllocPage(MemoryClass memoryClass, std::uint64_t size)
{
    std::uint32_t typeIndex = memoryClassTypes_[(int)memoryClass];
    if (typeIndex == TOOL_INVALID_ID)
    {
        std::uint32_t requiredFlags = 0;
        std::uint32_t preferredFlags = 0;
        std::uint32_t nonPreferredFlags = 0;

        switch (memoryClass)
        {
        case MemoryClass::DeviceFast:
            requiredFlags       = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            nonPreferredFlags   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case MemoryClass::CpuUniform:
            requiredFlags       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferredFlags      = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            nonPreferredFlags   = VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case MemoryClass::CpuStaging:
            requiredFlags       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferredFlags      = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            nonPreferredFlags   = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        case MemoryClass::CpuReadback:
            requiredFlags       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferredFlags      = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            nonPreferredFlags   = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            break;
        default:
            assert(false && "MemoryClass not implemented.");
        }

        typeIndex = memoryClassTypes_[(int)memoryClass] = FindBestMemoryType(requiredFlags, preferredFlags, nonPreferredFlags);
    }

    assert(typeIndex != TOOL_INVALID_ID && "Failed to find approprieate memory type.");

    VkMemoryAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.allocationSize = size;
    info.memoryTypeIndex = typeIndex;

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkAllocateMemory(device_->Handle(), &info, nullptr, &deviceMemory));

    VkMemoryPropertyFlags memoryFlags = device_->Properties().memoryProperties2.memoryProperties.memoryTypes[typeIndex].propertyFlags;

    void* mappedMemory = nullptr;
    if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VK_ASSERT(table_->vkMapMemory(device_->Handle(), deviceMemory, 0, VK_WHOLE_SIZE, VK_FLAGS_NONE, &mappedMemory));
    }

    MemoryPage* memory = new MemoryPage{};
    memory->deviceMemory_ = deviceMemory;
    memory->size_ = size;
    memory->memoryClass = memoryClass;
    memory->mappedMemoryPtr_ = mappedMemory;
    memory->bindCount_ = 0;
    memory->nextFreeOffset_ = 0;

    allocations_.emplace_back(memory);

    return memory;
}

void MemoryController::FreePage(MemoryPage* page)
{
    std::uint32_t deletedPageIndex = TOOL_INVALID_ID;

    std::uint32_t const allocationsCount = static_cast<std::uint32_t>(allocations_.size());
    for (auto i = 0u; i < allocationsCount; ++i) {
        if (page == allocations_[i]) {
            deletedPageIndex = i;
            break;
        }
    }

    if (deletedPageIndex == TOOL_INVALID_ID)
        assert(false && "MemoryController::FreePage can't delete the page.");

    table_->vkFreeMemory(device_->Handle(), page->deviceMemory_, nullptr);

    delete page;
    allocations_.erase(allocations_.begin() + deletedPageIndex);
}

}