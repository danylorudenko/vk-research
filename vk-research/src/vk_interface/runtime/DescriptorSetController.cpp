#include "DescriptorSetController.hpp"

#include <utility>
#include <cassert>

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\pipeline\DescriptorLayoutController.hpp>
#include <vk_interface\image\ImagesProvider.hpp>
#include <vk_interface\buffer\BuffersProvider.hpp>


namespace VKW
{

DescriptorSetController::DescriptorSetController()
    : table_{ nullptr }
    , device_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , pool_{ VK_NULL_HANDLE }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , layoutController_{ desc.layoutController_ }
    , pool_{ VK_NULL_HANDLE }
{
    VkDescriptorPoolSize poolSizes[] = {
        {
            VK_DESCRIPTOR_TYPE_SAMPLER,
            8u
        },
        {
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            64u
        },
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            64u
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            256u
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            16u
        }
    };

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1024;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
    poolInfo.pPoolSizes = poolSizes;

    VK_ASSERT(table_->vkCreateDescriptorPool(device_->Handle(), &poolInfo, nullptr, &pool_));
}

DescriptorSetController::DescriptorSetController(DescriptorSetController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , pool_{ VK_NULL_HANDLE }
{
	operator=(std::move(rhs));
}

DescriptorSetController& DescriptorSetController::operator=(DescriptorSetController&& rhs)
{
	std::swap(table_, rhs.table_);
	std::swap(device_, rhs.device_);
    std::swap(buffersProvider_, rhs.buffersProvider_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(layoutController_, rhs.layoutController_);
    std::swap(pool_, rhs.pool_);
	std::swap(descriptorSets_, rhs.descriptorSets_);

	return *this;
}

DescriptorSetController::~DescriptorSetController()
{
    for (auto& set : descriptorSets_) {
        delete set;
    }
    
    if (pool_ != VK_NULL_HANDLE) {
        table_->vkDestroyDescriptorPool(device_->Handle(), pool_, nullptr);
        pool_ = VK_NULL_HANDLE;
    }
}

DescriptorSetHandle DescriptorSetController::AllocDescriptorSet(DescriptorSetLayout const* layout)
{
    VkDescriptorSetLayout vkLayout = layout->handle_;

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = pool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &vkLayout;
    
    VkDescriptorSet vkSet = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkAllocateDescriptorSets(device_->Handle(), &allocInfo, &vkSet));

    auto* result = new DescriptorSet{};
    result->handle_ = vkSet;
    result->layout_ = layout;

    descriptorSets_.emplace_back(result);

    return DescriptorSetHandle{ result };
}

void DescriptorSetController::ReleaseDescriptorSet(DescriptorSetHandle handle)
{
    DescriptorSet* set = handle.handle_;
    VkDescriptorSet vkSet = set->handle_;

    VK_ASSERT(table_->vkFreeDescriptorSets(device_->Handle(), pool_, 1, &vkSet));

    delete set;
}

DescriptorSet* DescriptorSetController::GetDescriptorSet(DescriptorSetHandle handle)
{
    return handle.handle_;
}

}