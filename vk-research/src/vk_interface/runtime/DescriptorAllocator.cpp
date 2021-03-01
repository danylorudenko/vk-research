#include <vk_interface\runtime\DescriptorAllocator.hpp>

#include <cstdint>
#include <utility>

namespace VKW
{

DescriptorAllocator::DescriptorAllocator(ImportTable* table, Device* device)
    : table_{ table }
    , device_{ device }
{
    bufferHeap_.Init(table, device, HEAP_TYPE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DEFAULT_HEAP_SIZE);
    textureHeap_.Init(table, device, HEAP_TYPE_TEXTURE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DEFAULT_HEAP_SIZE);
}

DescriptorAllocator::DescriptorAllocator(DescriptorAllocator&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

DescriptorAllocator& DescriptorAllocator::operator=(DescriptorAllocator&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    std::swap(bufferHeap_, rhs.bufferHeap_);
    std::swap(textureHeap_, rhs.textureHeap_);

    return *this;
}

DescriptorAllocator::~DescriptorAllocator()
{
    bufferHeap_.Destroy(table_, device_);
    textureHeap_.Destroy(table_, device_);
}

DescriptorHandle DescriptorAllocator::AllocateBufferDescriptor()
{
    return bufferHeap_.Allocate();
}

DescriptorHandle DescriptorAllocator::AllocateTextureDescriptor()
{
    return textureHeap_.Allocate();
}

void DescriptorAllocator::FreeBufferDescriptor(DescriptorHandle& handle)
{
    assert(handle.heap_ == HEAP_TYPE_BUFFER);

    bufferHeap_.Free(handle);

    handle.id_ = 0;
    handle.count_ = 0;
    handle.heap_ = HEAP_TYPE_MAX;
}

void DescriptorAllocator::FreeTextureDescriptor(DescriptorHandle& handle)
{
    assert(handle.heap_ == HEAP_TYPE_TEXTURE);
    
    textureHeap_.Free(handle);

    handle.id_ = 0;
    handle.count_ = 0;
    handle.heap_ = HEAP_TYPE_MAX;
}



}

