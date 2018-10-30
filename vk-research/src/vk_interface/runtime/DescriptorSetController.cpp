#include "DescriptorSetController.hpp"
#include <utility>
#include <cassert>
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
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , layoutController_{ desc.layoutController_ }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
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
	std::swap(descriptorSets_, rhs.descriptorSets_);

	return *this;
}

DescriptorSetController::~DescriptorSetController()
{

}

void DescriptorSetController::AssembleSetCreateInfo(VkDescriptorSet dstSet, DescriptorSetDesc const& desc, VkWriteDescriptorSet* results)
{
    static DescriptorWriteInfo descriptorWriteInfo[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    

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
        
        switch (desc.members_[i].type_) {
        case DescriptorType::SAMPLED_TEXTURE:
        {
            ImageView* imageView = imagesProvider_->GetImageView(desc.members_[i].imageDesc.imageViewHandle_);
            DecorateImageViewWriteDesc(wSet, descriptorWriteInfo[i], imageView->handle_);
        }
            break;
        case DescriptorType::SAMPLER:
        {
            VkSampler defaultSampler = imagesProvider_->DefaultSamplerHandle();
            DecorateSamplerWriteDesc(wSet, descriptorWriteInfo[i], defaultSampler);
        }
            break;
        case DescriptorType::UNIFORM_BUFFER:
        {
            // TODO
            auto& bufferInfo = desc.members_[i].bufferInfo;
            BufferResource* buffer = nullptr;
            DecorateBufferWriteDesc(wSet, descriptorWriteInfo[i], buffer->handle_, bufferInfo.offset_, bufferInfo.size_);
        }
            break;
        default:
            assert(false && "Unsupported DescriptorType.");
        }
    }

}

void DescriptorSetController::DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkImageView view)
{

}

void DescriptorSetController::DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkSampler sampler)
{
    
}

void DescriptorSetController::DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkBufferView view)
{

}

void DescriptorSetController::DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size)
{

}

}