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
    AssignDefaultPageSizes();
}

MemoryController::MemoryController(MemoryControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    ClassifyDeviceMemoryTypes();
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
    defaultPageSizes_[(int)MemoryUsage::VERTEX_INDEX]             = 1024 * 1024;
    defaultPageSizes_[(int)MemoryUsage::UPLOAD_BUFFER]            = 1024 * 1024 * 5;
    defaultPageSizes_[(int)MemoryUsage::UNIFORM]                  = 1024 * 512;
    defaultPageSizes_[(int)MemoryUsage::SAMPLE_TEXTURE]           = 1024 * 1024 * 64;
    defaultPageSizes_[(int)MemoryUsage::DEPTH_STENCIL_ATTACHMENT] = 1024 * 1024 * 16;
    defaultPageSizes_[(int)MemoryUsage::COLOR_ATTACHMENT]         = 1024 * 1024 * 64;
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

void MemoryController::ClassifyDeviceMemoryTypes()
{
    VkPhysicalDeviceMemoryProperties2 const& memoryProperties2 = device_->Properties().memoryProperties2;
    VkPhysicalDeviceMemoryProperties const& memoryProperties = memoryProperties2.memoryProperties;
    VkPhysicalDeviceMemoryBudgetPropertiesEXT const* budgetProperties = (VkPhysicalDeviceMemoryBudgetPropertiesEXT*)memoryProperties2.pNext;

    std::uint32_t constexpr INVALID_ID      = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t deviceFastMemoryClassType  = INVALID_ID;
    std::uint32_t cpuUniformMemoryClassType  = INVALID_ID;
    std::uint32_t cpuStagingMemoryClassType  = INVALID_ID;
    std::uint32_t cpuReadbackClassType       = INVALID_ID;

    VkDeviceSize const typesCount = memoryProperties.memoryTypeCount;

    // MemoryClass::DeviceFastMemory
    for (std::uint32_t i = 0; i < typesCount; i++)
    {
        VkMemoryType const& memoryType = memoryProperties.memoryTypes[i];
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            if (deviceFastMemoryClassType == INVALID_ID)
            {
                deviceFastMemoryClassType = i;
                continue;
            }

            VkDeviceSize const thisTypeHeapSize = GetMemoryTypeBudget(*device_, i);
            VkDeviceSize const prevTypeHeapSize = GetMemoryTypeBudget(*device_, deviceFastMemoryClassType);

            std::uint32_t const prevTypeRelatedToHost = memoryProperties.memoryTypes[deviceFastMemoryClassType].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
            std::uint32_t const thisTypeRelatedToHost = memoryType.propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

            if (prevTypeRelatedToHost && !thisTypeRelatedToHost)
                deviceFastMemoryClassType = i;
        }
    }
    memoryClassTypes[(int)MemoryClass::DeviceFastMemory] = deviceFastMemoryClassType;

    // MemoryClass::CpuUniformMemory // better be coherent and non-cached
    for (std::uint32_t i = 0; i < typesCount; i++)
    {
        VkMemoryType const& memoryType = memoryProperties.memoryTypes[i];

        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            if (cpuUniformMemoryClassType == INVALID_ID)
            {
                cpuUniformMemoryClassType = i;
                continue;
            }

            //VkDeviceSize const thisTypeHeapSize = GetMemoryTypeBudget(*device_, i);
            //VkDeviceSize const prevTypeHeapSize = GetMemoryTypeBudget(*device_, cpuUniformMemoryClassType);

            std::uint32_t const prevTypeHostCoherent = memoryProperties.memoryTypes[cpuUniformMemoryClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            std::uint32_t const thisTypeHostCoherent = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            std::uint32_t const prevTypeHostCached = memoryProperties.memoryTypes[cpuUniformMemoryClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            std::uint32_t const thisTypeHostCached = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

            // most preferable: coherent and non-chached
            if ((!thisTypeHostCached && prevTypeHostCached) && (thisTypeHostCoherent && !prevTypeHostCoherent))
            {
                cpuUniformMemoryClassType = i;
                break;
            }
            // just coherent is better too
            if ((thisTypeHostCoherent && !prevTypeHostCoherent) || thisTypeHostCoherent)
            {
                cpuUniformMemoryClassType = i;
                continue;
            }
        }
    }
    memoryClassTypes[(int)MemoryClass::CpuUniformMemory] = cpuUniformMemoryClassType;

    // MemoryClass::CpuStagingMemory // better be coherent
    for (std::uint32_t i = 0; i < typesCount; i++)
    {
        VkMemoryType const& memoryType = memoryProperties.memoryTypes[i];

        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            if (cpuStagingMemoryClassType == INVALID_ID)
            {
                cpuStagingMemoryClassType = i;
                continue;
            }

            //VkDeviceSize const thisTypeHeapSize = GetMemoryTypeBudget(*device_, i);
            //VkDeviceSize const prevTypeHeapSize = GetMemoryTypeBudget(*device_, cpuUniformMemoryClassType);

            std::uint32_t const prevTypeHostCoherent = memoryProperties.memoryTypes[cpuStagingMemoryClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            std::uint32_t const thisTypeHostCoherent = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            std::uint32_t const prevTypeHostCached = memoryProperties.memoryTypes[cpuStagingMemoryClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            std::uint32_t const thisTypeHostCached = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

            // most preferable: coherent and non-chached
            if ((!thisTypeHostCached && prevTypeHostCached) && (thisTypeHostCoherent && !prevTypeHostCoherent))
            {
                cpuStagingMemoryClassType = i;
                break;
            }
            // just coherent is better too
            if ((thisTypeHostCoherent && !prevTypeHostCoherent) || thisTypeHostCoherent)
            {
                cpuStagingMemoryClassType = i;
                continue;
            }
        }
    }
    memoryClassTypes[(int)MemoryClass::CpuStagingMemory] = cpuStagingMemoryClassType;

    // MemoryClass::CpuReadbackMemory // beter be cached
    for (std::uint32_t i = 0; i < typesCount; i++)
    {
        VkMemoryType const& memoryType = memoryProperties.memoryTypes[i];

        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            if (cpuReadbackClassType == INVALID_ID)
            {
                cpuReadbackClassType = i;
                continue;
            }

            //VkDeviceSize const thisTypeHeapSize = GetMemoryTypeBudget(*device_, i);
            //VkDeviceSize const prevTypeHeapSize = GetMemoryTypeBudget(*device_, cpuUniformMemoryClassType);

            std::uint32_t const prevTypeHostCoherent = memoryProperties.memoryTypes[cpuReadbackClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            std::uint32_t const thisTypeHostCoherent = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            std::uint32_t const prevTypeHostCached = memoryProperties.memoryTypes[cpuReadbackClassType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            std::uint32_t const thisTypeHostCached = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

            // most preferable: host-cached and non-coherent
            if ((thisTypeHostCached && !prevTypeHostCached) && (!thisTypeHostCoherent && prevTypeHostCoherent))
            {
                cpuReadbackClassType = i;
                break;
            }
            // just cached is okay too
            if ((thisTypeHostCached && !prevTypeHostCached) || thisTypeHostCached)
            {
                cpuReadbackClassType = i;
                continue;
            }
        }
    }
    memoryClassTypes[(int)MemoryClass::CpuReadbackMemory] = cpuReadbackClassType;
}

MemoryPage* MemoryController::GetPage(MemoryPageHandle handle)
{
    return handle.page_;
}

void MemoryController::ProvideMemoryRegion(MemoryPageRegionDesc const& desc, MemoryRegion& regionOut)
{
    MemoryAccessBits accessFlags = MemoryAccessBits::NONE;
    MemoryClass memoryClass = MemoryClass::MAX;

    switch (desc.usage_)
    {
    case MemoryUsage::VERTEX_INDEX:
    case MemoryUsage::SAMPLE_TEXTURE:
    case MemoryUsage::STORAGE:
    case MemoryUsage::DEPTH_STENCIL_ATTACHMENT:
    case MemoryUsage::COLOR_ATTACHMENT:
        accessFlags = MemoryAccessBits::GPU_LOCAL;
        memoryClass = MemoryClass::DeviceFastMemory;
        break;
    case MemoryUsage::UPLOAD_BUFFER:
    case MemoryUsage::UNIFORM:
        accessFlags = MemoryAccessBits::CPU_WRITE;
        memoryClass = MemoryClass::CpuUniformMemory;
        break;
    default:
        assert(false && "Unsupported MemoryUsage");
    }

    std::uint32_t constexpr INVALID_ALLOCATION = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t validAllocation = INVALID_ALLOCATION;
    auto const allocationsCount = allocations_.size();
    for (auto i = 0u; i < allocationsCount; ++i) {
        auto const& page = allocations_[i];
        auto const accessValid  = (page->accessFlags_ & accessFlags) == accessFlags;
        auto const sizeValid    = desc.size_ <= page->GetFreeMemorySize();
        auto const usageValid   = desc.usage_ == page->usage_; // TODO: so many usage types, but actually we're just going to two flag combinations

        if (accessValid && sizeValid && usageValid) {
            validAllocation = i;
            break;
        }
    }

    if (validAllocation != INVALID_ALLOCATION) {
        GetNextFreePageRegion(MemoryPageHandle{ allocations_[validAllocation] }, desc, regionOut);
    }
    else {
        auto const defaultPageSize = defaultPageSizes_[(int)desc.usage_];
        auto const requestedSize = desc.size_ + desc.alignment_;
        auto const pageSize = requestedSize > defaultPageSize ? requestedSize : defaultPageSize;

        MemoryPageHandle newPage = AllocPage(accessFlags, desc.usage_, pageSize);
        GetNextFreePageRegion(newPage, desc, regionOut);
    }
}

void MemoryController::GetNextFreePageRegion(MemoryPageHandle pageHandle, MemoryPageRegionDesc const& desc, MemoryRegion& regionOut)
{
    std::uint64_t const size = desc.size_ + desc.alignment_;

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

    std::uint32_t const memoryTypesCount = device_->Properties().memoryProperties2.memoryProperties.memoryTypeCount;
    VkMemoryType const* memoryTypes = device_->Properties().memoryProperties2.memoryProperties.memoryTypes;

    std::uint32_t typeIndex = VK_MAX_MEMORY_TYPES;
    for (std::uint32_t i = 0u; i < memoryTypesCount; ++i) {
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


    void* mappedMemory = nullptr;
    if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VK_ASSERT(table_->vkMapMemory(device_->Handle(), deviceMemory, 0, VK_WHOLE_SIZE, VK_FLAGS_NONE, &mappedMemory));
    }

    MemoryPage* memory = new MemoryPage{};
    memory->deviceMemory_ = deviceMemory;
    memory->size_ = info.allocationSize;
    memory->memoryTypeId_ = typeIndex;
    memory->propertyFlags_ = memoryTypes[typeIndex].propertyFlags;
    memory->accessFlags_ = accessFlags;
    memory->usage_ = usage;
    memory->mappedMemoryPtr_ = mappedMemory;
    
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