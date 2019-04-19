#include "ResourceRendererProxy.hpp"
#include "ImportTable.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "memory\MemoryController.hpp"
#include "buffer\BuffersProvider.hpp"
#include "image\ImagesProvider.hpp"
#include "pipeline\DescriptorLayoutController.hpp"
#include "runtime\DescriptorSetController.hpp"
#include "pipeline\RenderPassController.hpp"
#include "resources\FramebufferController.hpp"
#include "runtime\FramedDescriptorsHub.hpp"

#include <utility>
#include <cassert>

namespace VKW
{

ResourceRendererProxy::ResourceRendererProxy()
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
    , resourcesController_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , renderPassController_{ nullptr }
    , framebufferController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxyDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , memoryController_{ desc.memoryController_ }
    , resourcesController_{ desc.resourcesController_ }
    , buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , layoutController_{ desc.layoutController_ }
    , descriptorSetsController_{ desc.descriptorSetsController_ }
    , renderPassController_{ desc.renderPassController_ }
    , framebufferController_{ desc.framebufferController_ }
    , framedDescriptorsHub_{ desc.framedDescriptorsHub_ }
{
    
}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxy&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
    , resourcesController_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , renderPassController_{ nullptr }
    , framebufferController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{
    operator=(std::move(rhs));
}

ResourceRendererProxy& ResourceRendererProxy::operator=(ResourceRendererProxy&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(memoryController_, rhs.memoryController_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(buffersProvider_, rhs.buffersProvider_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(layoutController_, rhs.layoutController_);
    std::swap(descriptorSetsController_, rhs.descriptorSetsController_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(framebufferController_, rhs.framebufferController_);
    std::swap(framedDescriptorsHub_, rhs.framedDescriptorsHub_);

    return *this;
}

ResourceRendererProxy::~ResourceRendererProxy()
{

}

std::uint32_t ResourceRendererProxy::FramesCount() const
{
    return framedDescriptorsHub_->framesCount_;
}

ProxyImageHandle ResourceRendererProxy::RegisterSwapchainImageViews()
{
    std::uint32_t const id = framedDescriptorsHub_->imageViewsNextId_++;
    std::uint32_t const buffering = framedDescriptorsHub_->framesCount_;
    for (auto i = 0u; i < buffering; ++i) {
        SwapchainImageViewDesc imDesc;
        imDesc.index_ = i;

        ImageViewHandle viewHandle = imagesProvider_->RegisterSwapchainImageView(imDesc);
        framedDescriptorsHub_->contexts_[i].imageViews_.push_back(viewHandle);
    }

    return ProxyImageHandle{ id };
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

    std::uint32_t id = framedDescriptorsHub_->descriptorSetsNextId_++;

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

    return ProxySetHandle{ id };
}


struct ResourceRendererProxy::DescriptorWriteData
{
    VkDescriptorImageInfo imageInfo;
    VkDescriptorBufferInfo bufferInfo;
    VkBufferView bufferView;
};

void ResourceRendererProxy::WriteSet(ProxySetHandle setHandle, ProxyDescriptorWriteDesc* descriptions)
{
    static DescriptorWriteData descriptorData[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    static VkWriteDescriptorSet writeDescritorSets[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];

    bool framedSet = false;
    if (framedDescriptorsHub_->framesCount_ > 1) {
        if (framedDescriptorsHub_->contexts_[0].descriptorSets_[setHandle.id_].handle_ != framedDescriptorsHub_->contexts_[1].descriptorSets_[setHandle.id_].handle_) {
            framedSet = true;
        }
    }


    DescriptorSetHandle firstFrameSetHandle = framedDescriptorsHub_->contexts_[0].descriptorSets_[setHandle.id_];
    DescriptorSet* firstFrameSet = descriptorSetsController_->GetDescriptorSet(firstFrameSetHandle);
    DescriptorSetLayout* layout = layoutController_->GetDescriptorSetLayout(firstFrameSet->layout_);

    auto const setMembersCount = layout->membersCount_;

    if (framedSet) {
        auto const framesCount = framedDescriptorsHub_->framesCount_;
        
        DescriptorSet* framedSets[FramedDescriptorsHub::MAX_FRAMES_COUNT];
        framedSets[0] = firstFrameSet;

        for (auto i = 1u; i < framesCount; ++i) {
            DescriptorSetHandle descriptorSetHandle = framedDescriptorsHub_->contexts_[i].descriptorSets_[setHandle.id_];
            DescriptorSet* set = descriptorSetsController_->GetDescriptorSet(descriptorSetHandle);
            framedSets[i] = set;
        }

        VkDevice const device = device_->Handle();
        for (auto i = 0u; i < framesCount; ++i) { // each frame
            VkDescriptorSet const targetSet = framedSets[i]->handle_;
            for (auto j = 0u; j < setMembersCount; ++j) { // each member
                VkWriteDescriptorSet& wds = writeDescritorSets[j];
                wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                wds.pNext = nullptr;
                wds.dstSet = targetSet;
                wds.dstBinding = layout->membersInfo_[j].binding_;
                wds.dstArrayElement = 0;
                wds.descriptorCount = 1;
                wds.descriptorType = layout->membersInfo_[j].type_;
                // actual data for descriptors goes next
                switch (wds.descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                {
                    ImageView* imageView = imagesProvider_->GetImageView(descriptions[j].frames_[i].imageDesc_.imageViewHandle_);
                    DecorateImageViewWriteDesc(wds, descriptorData[j], imageView->handle_);
                }
                break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                {
                    VkSampler defaultSampler = imagesProvider_->DefaultSamplerHandle();
                    DecorateSamplerWriteDesc(wds, descriptorData[j], defaultSampler);
                }
                break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                {
                    auto& bufferInfo = descriptions[j].frames_[i].bufferInfo_;
                    BufferResource* bufferResource = buffersProvider_->GetViewResource(bufferInfo.pureBufferViewHandle_);
                    DecorateBufferWriteDesc(wds, descriptorData[j], bufferResource->handle_, bufferInfo.offset_, bufferInfo.size_);
                }
                break;
                default:
                    assert(false && "Unsupported DescriptorType.");
                }
            }

            table_->vkUpdateDescriptorSets(
                device,
                layout->membersCount_,
                writeDescritorSets,
                0,
                nullptr);
        }
    }
    else {
        VkDescriptorSet const targetSet = firstFrameSet->handle_;

        for (auto i = 0u; i < setMembersCount; ++i) {
            VkWriteDescriptorSet& wds = writeDescritorSets[i];
            wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wds.pNext = nullptr;
            wds.dstSet = targetSet;
            wds.dstBinding = layout->membersInfo_[i].binding_;
            wds.dstArrayElement = 0;
            wds.descriptorCount = 1;
            wds.descriptorType = layout->membersInfo_[i].type_;
            // actual data for descriptors goes next
            switch (wds.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            {
                ImageView* imageView = imagesProvider_->GetImageView(descriptions[i].frames_[0].imageDesc_.imageViewHandle_);
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
                auto& bufferInfo = descriptions[i].frames_[0].bufferInfo_;
                BufferResource* bufferResource = buffersProvider_->GetViewResource(bufferInfo.pureBufferViewHandle_);
                DecorateBufferWriteDesc(wds, descriptorData[i], bufferResource->handle_, bufferInfo.offset_, bufferInfo.size_);
            }
            break;
            default:
                assert(false && "Unsupported DescriptorType.");
            }
        }

        table_->vkUpdateDescriptorSets(
            device_->Handle(),
            setMembersCount,
            writeDescritorSets,
            0,
            nullptr);
    }
}

DescriptorSetHandle ResourceRendererProxy::GetDescriptorSetHandle(ProxySetHandle handle, std::uint32_t context)
{
    return framedDescriptorsHub_->contexts_[context].descriptorSets_[handle.id_];
}

DescriptorSet* ResourceRendererProxy::GetDescriptorSet(ProxySetHandle handle, std::uint32_t context)
{
    DescriptorSetHandle setHandle = framedDescriptorsHub_->contexts_[context].descriptorSets_[handle.id_];
    return descriptorSetsController_->GetDescriptorSet(setHandle);
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

ProxyBufferHandle ResourceRendererProxy::CreateBuffer(BufferViewDesc const& decs)
{
    auto const framesCount = framedDescriptorsHub_->framesCount_;
    auto const id = framedDescriptorsHub_->bufferViewsNextId_++;

    BufferViewDesc viewDescs[FramedDescriptorsHub::MAX_FRAMES_COUNT];
    BufferViewHandle views[FramedDescriptorsHub::MAX_FRAMES_COUNT];


    for (auto i = 0u; i < framesCount; ++i) {
        viewDescs[i] = decs;
    }
    
    buffersProvider_->AcquireViews(framesCount, viewDescs, views);

    for (auto i = 0u; i < framesCount; ++i) {
        assert(framedDescriptorsHub_->contexts_[i].bufferViews_.size() == id && "Unsyncronized write to FramedDescriptors::bufferViews_.");
        framedDescriptorsHub_->contexts_[i].bufferViews_.emplace_back(views[i]);
    }

    return ProxyBufferHandle{ id };
}

BufferView* ResourceRendererProxy::GetBufferView(ProxyBufferHandle handle, std::uint32_t context)
{
    BufferViewHandle bufferViewHandle = framedDescriptorsHub_->contexts_[context].bufferViews_[handle.id_];
    return buffersProvider_->GetView(bufferViewHandle);
}

BufferViewHandle ResourceRendererProxy::GetBufferViewHandle(ProxyBufferHandle handle, std::uint32_t context)
{
    return framedDescriptorsHub_->contexts_[context].bufferViews_[handle.id_];
}

ProxyImageHandle ResourceRendererProxy::CreateImage(ImageViewDesc const& desc)
{
    ImageViewHandle imageHandle = imagesProvider_->AcquireImageView(desc);

    auto const id = framedDescriptorsHub_->imageViewsNextId_++;
    auto const framesCount = framedDescriptorsHub_->framesCount_;
    for (auto i = 0u; i < framesCount; ++i) {
        assert(framedDescriptorsHub_->contexts_[i].imageViews_.size() == id && "Unsyncronized write to FramedDescriptors::bufferViews_.");
        framedDescriptorsHub_->contexts_[i].imageViews_.emplace_back(imageHandle);
    }
    
    return ProxyImageHandle{ id };
}

ImageView* ResourceRendererProxy::GetImageView(ProxyImageHandle handle, std::uint32_t context)
{
    ImageViewHandle imageViewHandle = framedDescriptorsHub_->contexts_[context].imageViews_[handle.id_];
    return imagesProvider_->GetImageView(imageViewHandle);
}

ProxyFramebufferHandle ResourceRendererProxy::CreateFramebuffer(ProxyFramebufferDesc const& desc)
{
    RenderPass const* renderPass = renderPassController_->GetRenderPass(desc.renderPass_);
    std::uint32_t const renderPassColorAttachmentsCount = renderPass->colorAttachmentsCount_;

    std::uint32_t const id = framedDescriptorsHub_->framebuffersNextId_++;
    std::uint32_t const framesCount = framedDescriptorsHub_->framesCount_;

    for (auto i = 0u; i < framesCount; ++i) {
        auto& frameResources = framedDescriptorsHub_->contexts_[i];
        assert(frameResources.framebuffers_.size() == id && "Unsynchronized access to FramedDescriptors::framebuffers_");
        
        ImageViewHandle colorAttachmentHandles[RenderPass::MAX_COLOR_ATTACHMENTS];
        for (auto j = 0u; j < renderPassColorAttachmentsCount; ++j) {
            colorAttachmentHandles[j] = frameResources.imageViews_[desc.colorAttachments_[j].id_];
        }

        ImageViewHandle* depthStancilAttachmentHandle = nullptr;
        if (renderPass->depthStencilAttachmentInfo_.usage_ == RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL) {
            ProxyImageHandle* depthStencilProxyHandle = desc.depthStencilAttachment_;
            assert(depthStancilAttachmentHandle != nullptr && "ResourceRendererProxy::CreateFramebuffer: null depth stencil attachment handle!)");
            depthStancilAttachmentHandle = &frameResources.imageViews_[depthStencilProxyHandle->id_];
        }

        FramebufferDesc vkFBDesc;
        vkFBDesc.renderPass_ = desc.renderPass_;
        vkFBDesc.width_ = desc.width_;
        vkFBDesc.height_ = desc.height_;
        vkFBDesc.colorAttachments_ = colorAttachmentHandles;
        vkFBDesc.depthStencilAttachment_ = depthStancilAttachmentHandle;

        FramebufferHandle framebufferHandle = framebufferController_->CreateFramebuffer(vkFBDesc);

        frameResources.framebuffers_.emplace_back(framebufferHandle);
    }

    return ProxyFramebufferHandle{ id };
}

Framebuffer* ResourceRendererProxy::GetFramebuffer(ProxyFramebufferHandle handle, std::uint32_t context)
{
    FramebufferHandle framebufferHandle = framedDescriptorsHub_->contexts_[context].framebuffers_[handle.id_];
    return framebufferController_->GetFramebuffer(framebufferHandle);
}

VKW::BufferResource* ResourceRendererProxy::GetResource(VKW::BufferResourceHandle handle)
{
    return resourcesController_->GetBuffer(handle);
}

VKW::ImageResource* ResourceRendererProxy::GetResource(VKW::ImageResourceHandle handle)
{
    return resourcesController_->GetImage(handle);
}

VKW::MemoryPage* ResourceRendererProxy::GetMemoryPage(VKW::BufferResourceHandle handle)
{
    VKW::BufferResource* resource = GetResource(handle);
    return memoryController_->GetPage(resource->memory_.pageHandle_);
}

void* ResourceRendererProxy::MapBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context)
{
    VKW::BufferView const* view = GetBufferView(handle, context);
    VKW::BufferResource const* resource = GetResource(view->providedBuffer_->bufferResource_);
    VKW::MemoryPage const* memoryPage = GetMemoryPage(view->providedBuffer_->bufferResource_);

    std::uint64_t const mappingOffset = resource->memory_.offset_ + view->offset_;

    void* result = nullptr;
    VK_ASSERT(table_->vkMapMemory(device_->Handle(), memoryPage->deviceMemory_, mappingOffset, view->size_, VK_FLAGS_NONE, &result));

    return result;
}

void ResourceRendererProxy::FlushBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context)
{
    VKW::BufferView const* view = GetBufferView(handle, context);
    VKW::BufferResource const* resource = GetResource(view->providedBuffer_->bufferResource_);
    VKW::MemoryPage const* memoryPage = GetMemoryPage(view->providedBuffer_->bufferResource_);

    std::uint64_t const mappingOffset = resource->memory_.offset_ + view->offset_;

    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = memoryPage->deviceMemory_;
    range.offset = mappingOffset;
    range.size = view->size_;

    VK_ASSERT(table_->vkFlushMappedMemoryRanges(device_->Handle(), 1, &range));
}

}