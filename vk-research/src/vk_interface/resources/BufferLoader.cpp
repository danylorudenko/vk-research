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

BufferLoader::BufferLoader(VulkanImportTable* table, Device* device)
    : table_{ table }
    , device_{ device }
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

    return *this;
}

Buffer BufferLoader::LoadBuffer(BufferCreateInfo const& desc)
{
    assert(IsPowerOf2(desc.alignment_) && "Alignemnt is not power of 2!");

    VkBufferCreateInfo vkBufferCreateInfo;
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = nullptr;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCreateInfo.size = desc.size_;
    vkBufferCreateInfo.flags = VK_FLAGS_NONE;
    vkBufferCreateInfo.usage = VK_FLAGS_NONE;

    MemoryPageRegionDesc regionDesc;
    regionDesc.size_ = desc.size_;
    regionDesc.alignment_ = desc.alignment_;

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


    MemoryPageRegion memoryRegion;
    memoryRegion.page_ = nullptr;
    memoryRegion.size_ = 0;
    memoryRegion.offset_ = 0;

    device_->MemoryController().ProvideMemoryPageRegion(regionDesc, memoryRegion);
    assert(memoryRegion.page_ != nullptr && "Couldn't provide memory region for the buffer.");

    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), vkBuffer, memoryRegion.page_->deviceMemory_, memoryRegion.offset_));


    Buffer result;
    result.handle_ = vkBuffer;

    return result;
}

void BufferLoader::UnloadBuffer(Buffer& buffer)
{
    table_->vkDestroyBuffer(device_->Handle(), buffer.handle_, nullptr);
}

BufferLoader::~BufferLoader()
{
}

}