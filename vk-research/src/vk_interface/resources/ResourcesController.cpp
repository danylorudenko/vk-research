#include <utility>
#include "ResourcesController.hpp"
#include "../memory/MemoryController.hpp"
#include "../Device.hpp"
#include "../Tools.hpp"

namespace VKW
{

ResourcesController::ResourcesController()
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
{
}

ResourcesController::ResourcesController(ResourcesControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , memoryController_{ desc.memoryController_ }
{

}

ResourcesController::ResourcesController(ResourcesController&& rhs)
{
    operator=(std::move(rhs));
}

ResourcesController& ResourcesController::operator=(ResourcesController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(memoryController_, rhs.memoryController_);

    std::swap(staticBuffers_, rhs.staticBuffers_);
    std::swap(staticImages_, rhs.staticImages_);
    
    return *this;
}

ResourcesController::~ResourcesController()
{
    for (auto& staticBuffer : staticBuffers_) {
        auto memory = staticBuffer.memory_;
        table_->vkDestroyBuffer(device_->Handle(), staticBuffer.handle_, nullptr);
        memoryController_->ReleaseMemoryRegion(staticBuffer.memory_);
    }
}

BufferResourceHandle ResourcesController::CreateBuffer(BufferDesc const& desc)
{
    VkBufferCreateInfo vkBufferCreateInfo;
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = nullptr;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCreateInfo.size = desc.size_;
    vkBufferCreateInfo.flags = VK_FLAGS_NONE;
    vkBufferCreateInfo.usage = VK_FLAGS_NONE; // temp


    MemoryPageRegionDesc regionDesc;

    switch (desc.usage_)
    {
    case BufferUsage::VERTEX_INDEX:
        regionDesc.usage_ = MemoryUsage::VERTEX_INDEX;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        break;
    case BufferUsage::UNIFORM:
        regionDesc.usage_ = MemoryUsage::UNIFORM;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        break;
    case BufferUsage::UPLOAD_BUFFER:
        regionDesc.usage_ = MemoryUsage::UPLOAD_BUFFER;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        break;
    }


    VkBuffer vkBuffer = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateBuffer(device_->Handle(), &vkBufferCreateInfo, nullptr, &vkBuffer));

    VkMemoryRequirements memoryRequirements;
    table_->vkGetBufferMemoryRequirements(device_->Handle(), vkBuffer, &memoryRequirements);

    regionDesc.size_ = memoryRequirements.size;
    regionDesc.alignment_ = memoryRequirements.alignment;
    regionDesc.memoryTypeBits_ = memoryRequirements.memoryTypeBits;
    assert(IsPowerOf2(regionDesc.alignment_) && "Alignemnt is not power of 2!");


    MemoryRegion memoryRegion = { { 0 }, 0, 0 };
    memoryController_->ProvideMemoryRegion(regionDesc, memoryRegion);
    auto const& page = memoryController_->GetPage(memoryRegion.pageHandle_);
    VkDeviceMemory deviceMemory = page.deviceMemory_;

    assert(memoryRequirements.memoryTypeBits & (1 << page.memoryTypeId_) && "MemoryRegion has invalid memoryType");

    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), vkBuffer, deviceMemory, memoryRegion.offset_));

    staticBuffers_.emplace_back(vkBuffer, static_cast<std::uint32_t>(desc.size_), memoryRegion);

    return { static_cast<std::uint32_t>(staticBuffers_.size()) - 1 };
}

}