#include "BufferLoader.hpp"
#include "..\Device.hpp"
#include "..\Tools.hpp"

namespace VKW
{

BufferLoader::BufferLoader()
    : table_{ nullptr }
    , device_{ nullptr }
{
}

BufferLoader::BufferLoader(BufferLoaderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , memoryController_{ desc.memoryController_ }
{
}

BufferLoader::BufferLoader(BufferLoader&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

BufferLoader& BufferLoader::operator=(BufferLoader&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(memoryController_, rhs.memoryController_);

    return *this;
}

Buffer BufferLoader::LoadBuffer(BufferCreateInfo const& desc)
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

    Buffer buffer;
    buffer.handle_ = vkBuffer;

    VkMemoryRequirements memoryRequirements;
    table_->vkGetBufferMemoryRequirements(device_->Handle(), vkBuffer, &memoryRequirements);

    regionDesc.size_ = memoryRequirements.size;
    regionDesc.alignment_ = memoryRequirements.alignment;

    assert(IsPowerOf2(regionDesc.alignment_) && "Alignemnt is not power of 2!");

    MemoryPageRegion memoryRegion;
    memoryRegion.page_ = nullptr;
    memoryRegion.size_ = 0;
    memoryRegion.offset_ = 0;

    memoryController_->ProvideMemoryPageRegion(regionDesc, memoryRegion);
    VkDeviceMemory deviceMemory = memoryRegion.page_->deviceMemory_;

    VkDeviceSize commitmentTest = 0;
    table_->vkGetDeviceMemoryCommitment(device_->Handle(), deviceMemory, &commitmentTest);

    assert(memoryRegion.page_ != nullptr && "Couldn't provide memory region for the buffer.");
    assert(memoryRequirements.memoryTypeBits & (1 << memoryRegion.page_->memoryTypeId_) && "MemoryPageRegion has invalid memoryType");
    
    
    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), vkBuffer, deviceMemory, memoryRegion.offset_));

    return buffer;
}

void BufferLoader::UnloadBuffer(Buffer& buffer)
{
    table_->vkDestroyBuffer(device_->Handle(), buffer.handle_, nullptr);
}

BufferLoader::~BufferLoader()
{
}

}