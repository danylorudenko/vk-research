#include "ResourceRendererProxy.hpp"

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Swapchain.hpp>
#include <vk_interface\memory\MemoryController.hpp>
#include <vk_interface\buffer\BuffersProvider.hpp>
#include <vk_interface\image\ImagesProvider.hpp>
#include <vk_interface\pipeline\DescriptorLayoutController.hpp>
#include <vk_interface\runtime\DescriptorSetController.hpp>
#include <vk_interface\pipeline\RenderPassController.hpp>
#include <vk_interface\resources\FramebufferController.hpp>
#include <vk_interface\runtime\FramedDescriptorsHub.hpp>

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

ProxyImageHandle ResourceRendererProxy::RegisterSwapchainImageViews(Swapchain* swapchain)
{
    std::uint32_t const id = framedDescriptorsHub_->imageViewsNextId_++;
    std::uint32_t const buffering = framedDescriptorsHub_->framesCount_;
    for (auto i = 0u; i < buffering; ++i) {
        SwapchainImageViewDesc imDesc;
        imDesc.index_ = i;

        ImageView* view = imagesProvider_->RegisterSwapchainImageView(imDesc, swapchain);
        framedDescriptorsHub_->contexts_[i].imageViews_.push_back(view);
    }

    return ProxyImageHandle{ id };
}

ProxySetHandle ResourceRendererProxy::CreateSet(DescriptorSetLayout* layout)
{
    // Here, based on the contents of the layout i should decide, 
    // wheather the descriptorset should be framed

    std::uint32_t const layoutMembersCount = layout->membersCount_;

    bool framedSet = false;
    for (auto i = 0u; i < layoutMembersCount; ++i) {

        auto& layoutMember = layout->membersInfo_[i];

        switch (layoutMember.type_) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            framedSet = true;
            break;

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
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
    // TODO: ANNNNNNNNNIHILATE THIS SHITTY DUPLICATION
    // PURGE THE HERECY
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
    DescriptorSetLayout* layout = firstFrameSet->layout_;

    auto const setMembersCount = layout->membersCount_;

    if (framedSet) {
        auto const framesCount = framedDescriptorsHub_->framesCount_;
        
        DescriptorSet* framedSets[CONSTANTS::MAX_FRAMES_BUFFERING];
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
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                {
                    ImageView* imageView = descriptions[j].frames_[i].imageDesc_.imageView_;
                    VkSampler sampler = descriptions[j].frames_[0].imageDesc_.sampler_;

                    DecorateImageViewWriteDesc(wds, descriptorData[j], imageView->handle_, descriptions[j].frames_[i].imageDesc_.layout_);
                    DecorateSamplerWriteDesc(wds, descriptorData[j], sampler);
                }
                break;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                {
                    ImageView* imageView = descriptions[j].frames_[i].imageDesc_.imageView_;
                    DecorateImageViewWriteDesc(wds, descriptorData[j], imageView->handle_, descriptions[j].frames_[i].imageDesc_.layout_);
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
                    auto& bufferInfo = descriptions[j].frames_[i].pureBufferDesc_;
                    BufferResource* bufferResource = bufferInfo.bufferView_->bufferResource_;
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
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                ImageView* imageView = descriptions[i].frames_[0].imageDesc_.imageView_;
                VkSampler sampler = descriptions[i].frames_[0].imageDesc_.sampler_;

                DecorateImageViewWriteDesc(wds, descriptorData[i], imageView->handle_, descriptions[i].frames_[0].imageDesc_.layout_);
                DecorateSamplerWriteDesc(wds, descriptorData[i], sampler);
            }
            break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                ImageView* imageView = descriptions[i].frames_[0].imageDesc_.imageView_;
                DecorateImageViewWriteDesc(wds, descriptorData[i], imageView->handle_, descriptions[i].frames_[0].imageDesc_.layout_);
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
                auto& bufferInfo = descriptions[i].frames_[0].pureBufferDesc_;
                BufferResource* bufferResource = bufferInfo.bufferView_->bufferResource_;
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

void ResourceRendererProxy::DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkImageView view, VkImageLayout layout)
{
    dstInfo.imageInfo.imageView = view;
    dst.pImageInfo = &dstInfo.imageInfo;
    dstInfo.imageInfo.imageLayout = layout;
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
    BufferViewDesc viewDescs[CONSTANTS::MAX_FRAMES_BUFFERING];
    BufferView* views[CONSTANTS::MAX_FRAMES_BUFFERING];

    bool framedResource = false;
    switch (decs.usage_) {
    case BufferUsage::UNIFORM:
    case BufferUsage::VERTEX_INDEX_WRITABLE:
        framedResource = true;
    }

    std::uint32_t const framesCount = framedDescriptorsHub_->framesCount_;

    std::uint32_t const resourceFrames = framedResource ? framesCount : 1;
    for (std::uint32_t i = 0u; i < resourceFrames; ++i) {
        viewDescs[i] = decs;
    }
    
    buffersProvider_->CreateViewsAndCreateBuffers(resourceFrames, viewDescs, views);

    auto const proxyId = framedDescriptorsHub_->bufferViewsNextId_++;
    if (framedResource) {
        for (auto i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].bufferViews_.size() == proxyId && "Unsyncronized write to FramedDescriptors::bufferViews_.");
            framedDescriptorsHub_->contexts_[i].bufferViews_.emplace_back(views[i]);
        }
    }
    else {
        for (auto i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].bufferViews_.size() == proxyId && "Unsyncronized write to FramedDescriptors::bufferViews_.");
            framedDescriptorsHub_->contexts_[i].bufferViews_.emplace_back(views[0]); // we need only one buffer view instance if we have non-framed buffer
        }
    }
    
    return ProxyBufferHandle{ proxyId };
}

BufferView* ResourceRendererProxy::GetBufferView(ProxyBufferHandle handle, std::uint32_t context)
{
    return framedDescriptorsHub_->contexts_[context].bufferViews_[handle.id_];
}

ProxyImageHandle ResourceRendererProxy::CreateImage(ImageViewDesc const& desc)
{
    ImageViewDesc imageViewDescs[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    ImageView* imageViews[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    
    bool framedResource = false;
    switch (desc.usage_) {
    case VKW::ImageUsage::RENDER_TARGET:
    case VKW::ImageUsage::DEPTH:
    case VKW::ImageUsage::STENCIL:
    case VKW::ImageUsage::DEPTH_STENCIL:
    case VKW::ImageUsage::STORAGE_IMAGE:
        framedResource = true;
    }

    std::uint32_t const framesCount = framedDescriptorsHub_->framesCount_;
    std::uint32_t const requiredImagesCount = framedResource ? framesCount : 1;
    for (std::uint32_t i = 0; i < requiredImagesCount; ++i) {
        imageViewDescs[i] = desc;
    }

    imagesProvider_->CreateViewsAndCreateImages(requiredImagesCount, imageViewDescs, imageViews);
    
    auto const proxyId = framedDescriptorsHub_->imageViewsNextId_++;
    if (framedResource) {
        for (std::uint32_t i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].imageViews_.size() == proxyId && "Unsyncronized write to FramedDescriptors::bufferViews_.");
            framedDescriptorsHub_->contexts_[i].imageViews_.emplace_back(imageViews[i]);
        }
    }
    else {
        for (std::uint32_t i = 0u; i < framesCount; ++i) {
            assert(framedDescriptorsHub_->contexts_[i].imageViews_.size() == proxyId && "Unsyncronized write to FramedDescriptors::bufferViews_.");
            framedDescriptorsHub_->contexts_[i].imageViews_.emplace_back(imageViews[0]);
        }
    }
    
    return ProxyImageHandle{ proxyId };
}

ImageView* ResourceRendererProxy::GetImageView(ProxyImageHandle handle, std::uint32_t context)
{
    return framedDescriptorsHub_->contexts_[context].imageViews_[handle.id_];
}

VkSampler ResourceRendererProxy::GetDefaultSampler()
{
    return imagesProvider_->DefaultSamplerHandle();
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
        
        ImageView* colorAttachments[RenderPass::MAX_COLOR_ATTACHMENTS];
        for (auto j = 0u; j < renderPassColorAttachmentsCount; ++j) {
            colorAttachments[j] = frameResources.imageViews_[desc.colorAttachments_[j].id_];
        }

        ImageView* depthStancilAttachment = nullptr;
        if (renderPass->depthStencilAttachmentInfo_.usage_ == RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL) {
            ProxyImageHandle* depthStencilProxyHandle = desc.depthStencilAttachment_;
            depthStancilAttachment = frameResources.imageViews_[depthStencilProxyHandle->id_];
            assert(depthStancilAttachment != nullptr && "ResourceRendererProxy::CreateFramebuffer: null depth stencil attachment handle!)");
        }

        FramebufferDesc vkFBDesc;
        vkFBDesc.renderPass_ = desc.renderPass_;
        vkFBDesc.width_ = desc.width_;
        vkFBDesc.height_ = desc.height_;
        vkFBDesc.colorAttachments_ = colorAttachments;
        vkFBDesc.depthStencilAttachment_ = depthStancilAttachment;

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

void* ResourceRendererProxy::MapBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context)
{
    VKW::BufferView const* view = GetBufferView(handle, context);
    VKW::BufferResource const* resource = view->bufferResource_;
    VKW::MemoryPage const* memoryPage = view->bufferResource_->memoryRegion_.page_;

    std::uint64_t const mappingOffset = resource->memoryRegion_.offset_ + view->offset_;

    void* result = nullptr;
    VK_ASSERT(table_->vkMapMemory(device_->Handle(), memoryPage->deviceMemory_, mappingOffset, view->size_, VK_FLAGS_NONE, &result));

    return result;
}

void ResourceRendererProxy::FlushBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context)
{
    VKW::BufferView const* view = GetBufferView(handle, context);
    VKW::BufferResource const* resource = view->bufferResource_;
    VKW::MemoryPage const* memoryPage = view->bufferResource_->memoryRegion_.page_;

    std::uint64_t const mappingOffset = resource->memoryRegion_.offset_ + view->offset_;

    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = memoryPage->deviceMemory_;
    range.offset = mappingOffset;
    range.size = VK_WHOLE_SIZE;

    VK_ASSERT(table_->vkFlushMappedMemoryRanges(device_->Handle(), 1, &range));
}

}