#include "ResourceRendererProxy.hpp"
#include "buffer/BuffersProvider.hpp"
#include "image/ImagesProvider.hpp"
#include "pipeline/DescriptorLayoutController.hpp"
#include "runtime/DescriptorSetController.hpp"
#include "runtime/FramedDescriptorsHub.hpp"

#include <utility>
#include <cassert>

namespace VKW
{

ResourceRendererProxy::ResourceRendererProxy()
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxyDesc const& desc)
    : buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , layoutController_{ desc.layoutController_ }
    , descriptorSetsController_{ desc.descriptorSetsController_ }
    , framedDescriptorsHub_{ desc.framedDescriptorsHub_ }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxy&& rhs)
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{
    operator=(std::move(rhs));
}

ResourceRendererProxy& ResourceRendererProxy::operator=(ResourceRendererProxy&& rhs)
{
    std::swap(buffersProvider_, rhs.buffersProvider_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(descriptorSetsController_, rhs.descriptorSetsController_);
    std::swap(framedDescriptorsHub_, rhs.framedDescriptorsHub_);

    return *this;
}

ResourceRendererProxy::~ResourceRendererProxy()
{

}

ProxySetHandle ResourceRendererProxy::CreateSet(DescriptorSetLayoutHandle layout)
{
    // Here, based on the contents of the layout i should decide, 
    // wheather the descriptorset should be framed
    
    DescriptorSetLayout* layoutPtr = layoutController_->GetDescriptorSetLayout(layout);
    std::uint32_t const layoutMembersCount = layoutPtr->membersCount_;

    bool framedSet = false;
    for (auto i = 0u; i < layoutMembersCount; ++i) {

        auto& layoutMember = layoutPtr->membersInfo_[i];

        switch (layoutMember.type_) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            framedSet = true;
            break;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            break;

        default:
            assert(false && "Unsupported descriptor type.");
        }
    }

    auto id = framedDescriptorsHub_->nextFreeId_++;

    DescriptorSetDesc setDesc;
    setDesc.layout_ = layout;

    if (framedSet) {
        // create multiple sets and put in different frames
        auto const framesCount = framedDescriptorsHub_->framesCount_;
        for (auto i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].descriptorSets_.size() == id && "Unsyncronized write to FramedDescriptors::descriptorSets.");

            DescriptorSetHandle setHandle = descriptorSetsController_->AllocDescriptorSet(setDesc);
            framedDescriptorsHub_->contexts_[i].descriptorSets_.emplace_back(setHandle);
        }
    }
    else {
        // create one set and put in all frames
        DescriptorSetHandle setHandle = descriptorSetsController_->AllocDescriptorSet(setDesc);

        auto const framesCount = framedDescriptorsHub_->framesCount_;
        for (auto i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].descriptorSets_.size() == id && "Unsyncronized write to FramedDescriptors::descriptorSets.");

            framedDescriptorsHub_->contexts_[i].descriptorSets_.emplace_back(setHandle);
        }
    }

    return { id };
}


struct ResourceRendererProxy::DescriptorWriteData
{
    VkDescriptorImageInfo imageInfo;
    VkDescriptorBufferInfo bufferInfo;
    VkBufferView bufferView;
};

void ResourceRendererProxy::WriteSet(ProxySetHandle setHandle, DescriptorDesc* descriptions)
{
    static DescriptorWriteData descriptorData[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    static VkWriteDescriptorSet writeDescritorSets[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];

    bool framedSet = false;
    if (framedDescriptorsHub_->framesCount_ > 1) {
        if (framedDescriptorsHub_->contexts_[0].descriptorSets_[setHandle.id_].handle_ != framedDescriptorsHub_->contexts_[1].descriptorSets_[setHandle.id_].handle_) {
            framedSet = true;
        }
    }


    if (framedSet) {
        auto const framesCount = framedDescriptorsHub_->framesCount_;
        
        DescriptorSetHandle firstFrameSetHandle = framedDescriptorsHub_->contexts_[0].descriptorSets_[setHandle.id_];
        DescriptorSet* firstFrameSet = descriptorSetsController_->GetDescriptorSet(firstFrameSetHandle);
        DescriptorSetLayout* layout = layoutController_->GetDescriptorSetLayout(firstFrameSet->layout_);

        DescriptorSet* framedSets[FramedDescriptorsHub::MAX_FRAMES_COUNT];
        framedSets[0] = firstFrameSet;

        for (auto i = 1u; i < framesCount; ++i) {
            DescriptorSetHandle descriptorSetHandle = framedDescriptorsHub_->contexts_[i].descriptorSets_[setHandle.id_];
            DescriptorSet* set = descriptorSetsController_->GetDescriptorSet(descriptorSetHandle);
            framedSets[i] = set;
        }

        auto const setMembersCount = layout->membersCount_;
        for (auto i = 0u; i < framesCount; ++i) { // each frame
            for (auto j = 0u; j < setMembersCount; ++j) { // each member
                VkWriteDescriptorSet& wds = writeDescritorSets[j];
                wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                wds.pNext = nullptr;
                wds.dstSet = framedSets[i]->handle_;
                wds.dstBinding = layout->membersInfo_[j].binding_;
                wds.dstArrayElement = 0;
                wds.descriptorCount = 1;
                wds.descriptorType = layout->membersInfo_[j].type_;
                // actual data for descriptors goes next
                switch (wds.descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                {
                    ImageView* imageView = imagesProvider_->GetImageView(descriptions[j].imageDesc.imageViewHandle_);
                    DecorateImageViewWriteDesc(wds, descriptorData[i], imageView->handle_);
                }
                break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                {
                    VkSampler defaultSampler = imagesProvider_->DefaultSamplerHandle();
                    DecorateSamplerWriteDesc(wds, descriptorData[i], defaultSampler);
                }
                break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                {
                    auto& bufferInfo = descriptions[i].bufferInfo;
                    BufferResource* bufferResource = buffersProvider_->GetViewResource(bufferInfo.pureBufferViewHandle_);
                    DecorateBufferWriteDesc(wds, descriptorData[i], bufferResource->handle_, bufferInfo.offset_, bufferInfo.size_);
                }
                break;
                default:
                    assert(false && "Unsupported DescriptorType.");
                }
            }
        }

        // TODO actual writes
    }
    else {
        // TODO
    }
}

void ResourceRendererProxy::DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkImageView view)
{
    dstInfo.imageInfo.imageView = view;
    dst.pImageInfo = &dstInfo.imageInfo;
}

void ResourceRendererProxy::DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler)
{
    dstInfo.imageInfo.sampler = sampler;
    dst.pImageInfo = &dstInfo.imageInfo;
}

void ResourceRendererProxy::DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view)
{
    dstInfo.bufferView = view;
    dst.pTexelBufferView = &dstInfo.bufferView;
}

void ResourceRendererProxy::DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size)
{
    dstInfo.bufferInfo.buffer = buffer;
    dstInfo.bufferInfo.offset = offset;
    dstInfo.bufferInfo.range = size;

    dst.pBufferInfo = &dstInfo.bufferInfo;
}

}