#include "DynamicResourcesStorage.hpp"
#include "..\memory\MemoryController.hpp"
#include "..\Tools.hpp"
#include "..\Device.hpp"

#include <algorithm>

namespace VAL
{

DynamicResourceStorage::Storage::Storage(VkBuffer buffer, std::uint32_t size, std::uint32_t freeOffset, std::uint16_t subresourceCount)
    : buffer_{ buffer }
    , size_{ size }
    , freeOffset_{ freeOffset }
    , subresourcesCount_{ subresourceCount }
{

}

DynamicResourceStorage::DynamicResourceStorage()
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
    , defaultContainerSize_{ 0 }
    , storages_{}
{

}

DynamicResourceStorage::DynamicResourceStorage(DynamicResourceStorageDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , memoryController_{ desc.memoryController_ }
    , defaultContainerSize_{ desc.defaultSize_ }
    , storages_{}
{

}

DynamicResourceStorage::DynamicResourceStorage(DynamicResourceStorage&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
    , defaultContainerSize_{ 0 }
    , storages_{}
{
    operator=(std::move(rhs));
}

DynamicResourceStorage& DynamicResourceStorage::operator=(DynamicResourceStorage&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(memoryController_, rhs.memoryController_);
    std::swap(defaultContainerSize_, rhs.defaultContainerSize_);
    std::swap(storages_, rhs.storages_);

    return *this;
}

DynamicResourceStorage::~DynamicResourceStorage()
{
    for (auto storage : storages_) {
        table_->vkDestroyBuffer(device_->Handle(), storage.buffer_, nullptr);
    }
}

DynamicResourceStorage::StorageHandle DynamicResourceStorage::AllocateStorage(std::uint32_t size)
{
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.flags = VK_FLAGS_NONE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VkBuffer buffer = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateBuffer(device_->Handle(), &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    table_->vkGetBufferMemoryRequirements(device_->Handle(), buffer, &memRequirements);

    MemoryPageRegionDesc memoryDesc;
    memoryDesc.size_ = memRequirements.size;
    memoryDesc.alignment_ = memRequirements.alignment;
    memoryDesc.memoryTypeBits_ = memRequirements.memoryTypeBits;
    memoryDesc.usage_ = MemoryUsage::UNIFORM;
    
    MemoryRegion memory{ {}, 0, 0 };
    memoryController_->ProvideMemoryRegion(memoryDesc, memory);
    auto const* page = memoryController_->GetPage(memory.pageHandle_);

    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), buffer, page->deviceMemory_, memory.offset_));

    storages_.emplace_back(buffer, size, 0, 0);

    return { static_cast<std::uint32_t>(storages_.size()) - 1 };
}

void DynamicResourceStorage::FreeStorage(StorageHandle handle)
{
    table_->vkDestroyBuffer(device_->Handle(), storages_[handle.id_].buffer_, nullptr);
    storages_.erase(storages_.begin() + handle.id_);
}

void DynamicResourceStorage::AllocSubresourcePatch(std::uint32_t count, std::uint32_t size, SubbufferResource* output)
{
    std::uint32_t const totalSize = count * size;
    auto result = std::find_if(storages_.begin(), storages_.end(), [totalSize](Storage const& storage)
    {
        return totalSize <= storage.size_ - storage.freeOffset_;
    });

    for (auto i = 0u; i < count; ++i) {
        output[i].offset_ = result->freeOffset_ * count;
    }

    result->freeOffset_ += size * count;
    result->subresourcesCount_ += count;
}

void DynamicResourceStorage::FreeSubresourcePatch(std::uint32_t count, SubbufferResource* subbuffers)
{
    for (auto i = 0u; i < storages_.size(); i++) {
        if (storages_[i].buffer_ == subbuffers->handle_) {
            if ((storages_[i].subresourcesCount_ -= count) <= 0) {
                FreeStorage({ i });
            }
            return;
        }
    }
}

}