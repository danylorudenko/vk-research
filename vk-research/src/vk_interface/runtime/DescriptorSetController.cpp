#include "DescriptorSetController.hpp"
#include <utility>
#include <cassert>
#include "../ImportTable.hpp"
#include "../Device.hpp"
#include "../image/ImageView.hpp"
#include "../pipeline/DescriptorLayoutController.hpp"
#include "../image/ImagesProvider.hpp"
#include "../buffer/BuffersProvider.hpp"


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
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            256u
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

DescriptorSetHandle DescriptorSetController::AllocDescriptorSet(DescriptorSetDesc const& desc)
{
    DescriptorSetLayout* layout = layoutController_->GetDescriptorSetLayout(desc.layout_);
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
    result->layout_ = desc.layout_;
    result->handle_ = vkSet;

    descriptorSets_.emplace_back(result);

    // write to descriptors

    //static VkWriteDescriptorSet writeInfo[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    //AssembleSetCreateInfo(vkSet, desc, writeInfo);

    //table_->vkUpdateDescriptorSets(device_->Handle(),
    //    layout->membersCount_,
    //    writeInfo,
    //    0,
    //    nullptr);

    return DescriptorSetHandle{ result };
}

void DescriptorSetController::ReleaseDescriptorSet(DescriptorSetHandle handle)
{
    DescriptorSet* set = handle.handle_;
    VkDescriptorSet vkSet = set->handle_;

    VK_ASSERT(table_->vkFreeDescriptorSets(device_->Handle(), pool_, 1, &vkSet));

    delete set;
}

void DescriptorSetController::AssembleSetCreateInfo(VkDescriptorSet dstSet, DescriptorSetDesc const& desc, VkWriteDescriptorSet* results)
{
    static DescriptorWriteData descriptorData[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    
    DescriptorSetLayout* layout = layoutController_->GetDescriptorSetLayout(desc.layout_);

    auto const setMembersCount = layout->membersCount_;
    for (auto i = 0u; i < setMembersCount; ++i) {
        auto& wSet = results[i];
        wSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wSet.pNext = nullptr;
        wSet.dstSet = dstSet;
        wSet.dstBinding = layout->membersInfo_[i].binding_;
        wSet.dstArrayElement = 0;
        wSet.descriptorCount = 1;
        wSet.descriptorType = layout->membersInfo_[i].type_;
        
        //switch (wSet.descriptorType) {
        //case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        //{
        //    ImageView* imageView = imagesProvider_->GetImageView(desc.members_[i].imageDesc.imageViewHandle_);
        //    DecorateImageViewWriteDesc(wSet, descriptorData[i], imageView->handle_);
        //}
        //    break;
        //case VK_DESCRIPTOR_TYPE_SAMPLER:
        //{
        //    VkSampler defaultSampler = imagesProvider_->DefaultSamplerHandle();
        //    DecorateSamplerWriteDesc(wSet, descriptorData[i], defaultSampler);
        //}
        //    break;
        //case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        //{
        //    auto& bufferInfo = desc.members_[i].bufferInfo;
        //    BufferResource* bufferResource = buffersProvider_->GetViewResource(bufferInfo.pureBufferViewHandle_);
        //    DecorateBufferWriteDesc(wSet, descriptorData[i], bufferResource->handle_, bufferInfo.offset_, bufferInfo.size_);
        //}
        //    break;
        //default:
        //    assert(false && "Unsupported DescriptorType.");
        //}
    }

}

void DescriptorSetController::DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkImageView view)
{
    dstInfo.imageInfo.imageView = view;
    dst.pImageInfo = &dstInfo.imageInfo;
}

void DescriptorSetController::DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler)
{
    dstInfo.imageInfo.sampler = sampler;
    dst.pImageInfo = &dstInfo.imageInfo;
}

void DescriptorSetController::DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view)
{
    dstInfo.bufferView = view;
    dst.pTexelBufferView = &dstInfo.bufferView;
}

void DescriptorSetController::DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size)
{
    dstInfo.bufferInfo.buffer = buffer;
    dstInfo.bufferInfo.offset = offset;
    dstInfo.bufferInfo.range = size;

    dst.pBufferInfo = &dstInfo.bufferInfo;
}

}