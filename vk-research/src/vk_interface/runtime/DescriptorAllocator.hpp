#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>

#include <class_features\NonCopyable.hpp>

#include <vulkan\vulkan.h>

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Tools.hpp>
#include <vk_interface\runtime\ElementAllocator.hpp>

namespace VKW
{


enum HeapType
{
    HEAP_TYPE_BUFFER,
    HEAP_TYPE_TEXTURE,
    HEAP_TYPE_MAX
};

struct DescriptorHandle
{
    std::uint32_t   id_;
    std::uint32_t   count_;
    HeapType        heap_;
};


class Device;
class ImportTable;

class DescriptorAllocator
{
public:
    DescriptorAllocator(ImportTable* table, Device* device);

    DescriptorAllocator(DescriptorAllocator&& rhs);
    DescriptorAllocator& operator=(DescriptorAllocator&& rhs);

    DescriptorAllocator(DescriptorAllocator const&) = delete;
    DescriptorAllocator& operator=(DescriptorAllocator const&) = delete;

    ~DescriptorAllocator();

    DescriptorHandle AllocateBufferDescriptor();
    DescriptorHandle AllocateTextureDescriptor();

    void FreeBufferDescriptor(DescriptorHandle& handle);
    void FreeTextureDescriptor(DescriptorHandle& handle);

private:
    ImportTable* table_;
    Device* device_;

    template<typename TAllocator>
    struct DescriptorHeap
    {
        VkDescriptorSetLayout   layout_         = VK_NULL_HANDLE;
        VkDescriptorPool        pool_           = VK_NULL_HANDLE;
        VkDescriptorSet         set_            = VK_NULL_HANDLE;
        VkDescriptorType        descriptorType_ = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        HeapType                heapType_       = HEAP_TYPE_MAX;
        std::uint32_t           size_   = 0;
        bool                    isReset_= false;

        TAllocator              allocator_;

        void Init(ImportTable* table, Device* device, HeapType heapType, VkDescriptorType descriptorType, std::uint32_t size);
        void Reset(ImportTable* table, Device* device);
        void Destroy(ImportTable* table, Device* device);

        DescriptorHandle Allocate(std::uint16_t count = 1);
        void             Free(DescriptorHandle handle);
    };

    static constexpr std::uint16_t DEFAULT_HEAP_SIZE = 1000;

    DescriptorHeap<FreeListElementAllocator<DEFAULT_HEAP_SIZE>> bufferHeap_;
    DescriptorHeap<FreeListElementAllocator<DEFAULT_HEAP_SIZE>> textureHeap_;

};

template<typename TAllocator>
void DescriptorAllocator::DescriptorHeap<TAllocator>::Init(ImportTable* table, Device* device, HeapType heapType, VkDescriptorType descriptorType, std::uint32_t count)
{
    assert(set_ == VK_NULL_HANDLE);

    descriptorType_ = descriptorType;
    heapType_ = heapType;
    size_ = count;
    isReset_ = true;

    
    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorType = descriptorType;
    binding.descriptorCount = count;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flag =
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |         // can write descriptor after bound
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | // unknown size, must be last in set
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |           // not all descriptors must be valid on the whole set (unless they're used)
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;// can update descriptors in set that is in execution (unless that particular descriptor is used right now)

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags;
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.pNext = nullptr;
    bindingFlags.bindingCount = 1;
    bindingFlags.pBindingFlags = &flag;

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = &bindingFlags;
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;

    VK_ASSERT(table->vkCreateDescriptorSetLayout(device->Handle(), &createInfo, nullptr, &layout_));

    ///////////////////

    VkDescriptorPoolSize poolSize;
    poolSize.type = descriptorType;
    poolSize.descriptorCount = count;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    VK_ASSERT(table->vkCreateDescriptorPool(device->Handle(), &poolInfo, nullptr, &pool_));

    ///////////////////

    std::uint32_t countsData = count;
    VkDescriptorSetVariableDescriptorCountAllocateInfo counts;
    counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    counts.pNext = nullptr;
    counts.descriptorSetCount = 1;
    counts.pDescriptorCounts = &countsData;


    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &counts;
    allocInfo.descriptorPool = pool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout_;

    VK_ASSERT(table->vkAllocateDescriptorSets(device->Handle(), &allocInfo, &set_));
}

template<typename TAllocator>
void DescriptorAllocator::DescriptorHeap<TAllocator>::Reset(ImportTable* table, Device* device)
{
    assert(!isReset_ && "Resetting DescriptorHeap twice is not allowed.");

    VK_ASSERT(table->vkResetDescriptorPool(device->Handle(), pool_, VK_FLAGS_NONE));
}

template<typename TAllocator>
void DescriptorAllocator::DescriptorHeap<TAllocator>::Destroy(ImportTable* table, Device* device)
{
    VkDevice deviceHandle = device->Handle();

    table->vkDestroyDescriptorPool(deviceHandle, pool_, nullptr);           pool_ = VK_NULL_HANDLE;
    table->vkDestroyDescriptorSetLayout(deviceHandle, layout_, nullptr);    layout_ = VK_NULL_HANDLE;
}

template<typename TAllocator>
DescriptorHandle DescriptorAllocator::DescriptorHeap<TAllocator>::Allocate(std::uint16_t count)
{
    std::uint16_t const id = allocator_.Allocate(count);
    return DescriptorHandle{ id, count, heapType_ };
}

template<typename TAllocator>
void DescriptorAllocator::DescriptorHeap<TAllocator>::Free(DescriptorHandle handle)
{
    allocator_.Free(handle.id_);
}

}
