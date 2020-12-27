#include "Pass.hpp"

#include <renderer\Root.hpp>

#include <vk_interface/buffer/ProvidedBuffer.hpp>
#include <vk_interface\ResourceRendererProxy.hpp>
#include <vk_interface\pipeline\RenderPassController.hpp>
#include <vk_interface\pipeline\PipelineFactory.hpp>
#include <vk_interface\pipeline\Pipeline.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\worker\Worker.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>

#include <utility>

namespace Render
{

GraphicsPass::GraphicsPass()
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , pipelineFactory_{ nullptr }
    , width_{ 0 }
    , height_{ 0 }
{
}

GraphicsPass::GraphicsPass(GraphicsPassDesc const& desc)
    : root_{ desc.root_ }
    , table_{ desc.table_ }
    , device_{ desc.device_}
    , resourceProxy_{ desc.proxy_ }
    , renderPassController_{ desc.renderPassController_ }
    , pipelineFactory_{ desc.pipelineFactory_ }
    , descriptorLayoutController_{ nullptr }
    , width_{ desc.width_ }
    , height_{ desc.height_ }
{
    std::uint32_t const colorAttachmentCount = desc.colorAttachmentCount_;

    VKW::RenderPassAttachmentDesc colorAttachmentDescs[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    VKW::RenderPassAttachmentDesc depthStencilAttachmentDesc;

    VKW::RenderPassDesc vkRenderPassDesc;
    
    vkRenderPassDesc.colorAttachmentsCount_ = colorAttachmentCount;
    for (auto i = 0u; i < colorAttachmentCount; ++i) {

        VKW::ImageViewHandle imageViewHandle = desc.framedDescriptorsHub_->contexts_[0].imageViews_[desc.colorAttachments_[i].handle_.id_];
        VKW::ImageView* imageView = desc.imagesProvider_->GetImageView(imageViewHandle);
        colorAttachmentDescs[i].format_ = imageView->format_;
        colorAttachmentDescs[i].usage_ = desc.colorAttachments_[i].usage_;
    }

    vkRenderPassDesc.colorAttachments_ = colorAttachmentDescs;

    if (desc.depthStencilAttachment_ != nullptr) {
        VKW::ImageViewHandle imageViewHandle = desc.framedDescriptorsHub_->contexts_[0].imageViews_[desc.depthStencilAttachment_->id_];
        VKW::ImageView* imageView = desc.imagesProvider_->GetImageView(imageViewHandle);
        vkRenderPassDesc.depthStencilAttachment_ = &depthStencilAttachmentDesc;
        vkRenderPassDesc.depthStencilAttachment_->format_ = imageView->format_;
    }
    else {
        vkRenderPassDesc.depthStencilAttachment_ = nullptr;
    }

    vkRenderPass_ = renderPassController_->AssembleRenderPass(vkRenderPassDesc);




    VKW::ProxyFramebufferDesc framebufferProxyDesc;
    framebufferProxyDesc.renderPass_ = vkRenderPass_;
    framebufferProxyDesc.width_ = desc.width_;
    framebufferProxyDesc.height_ = desc.height_;

    for (auto j = 0u; j < desc.colorAttachmentCount_; ++j) {
        framebufferProxyDesc.colorAttachments_[j] = desc.colorAttachments_[j].handle_;
    }

    framebufferProxyDesc.depthStencilAttachment_ = desc.depthStencilAttachment_;
    
    framebuffer_ = resourceProxy_->CreateFramebuffer(framebufferProxyDesc);
}

GraphicsPass::GraphicsPass(GraphicsPass&& rhs)
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
    , width_{ 0 }
    , height_{ 0 }
{
    operator=(std::move(rhs));
}

GraphicsPass& GraphicsPass::operator=(GraphicsPass&& rhs)
{
    std::swap(root_, rhs.root_);
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(pipelineFactory_, rhs.pipelineFactory_);
    std::swap(descriptorLayoutController_, rhs.descriptorLayoutController_);
    std::swap(vkRenderPass_, rhs.vkRenderPass_);
    std::swap(framebuffer_, rhs.framebuffer_);
    std::swap(width_, rhs.width_);
    std::swap(height_, rhs.height_);
    std::swap(materialDelegatedData_, rhs.materialDelegatedData_);

    return *this;
}

GraphicsPass::~GraphicsPass()
{
    
}

VKW::RenderPassHandle GraphicsPass::VKWRenderPass() const
{
    return vkRenderPass_;
}

void GraphicsPass::RegisterMaterialData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey)
{
    materialDelegatedData_.emplace_back(materialKey, materialPerPassDataId, pipelineKey);
}

void GraphicsPass::Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    VKW::RenderPass* pass = renderPassController_->GetRenderPass(vkRenderPass_);
    VKW::Framebuffer* framebuffer = resourceProxy_->GetFramebuffer(framebuffer_, contextId);

    VkClearValue clearValues[VKW::RenderPass::MAX_ATTACHMENTS];
    std::uint32_t const colorAttachmentsCount = pass->colorAttachmentsCount_;
    for (auto i = 0u; i < colorAttachmentsCount; ++i) {

        // for now we only support floating attachments
        auto& val = clearValues[i];
        val.color.float32[0] = 0.0f;
        val.color.float32[1] = 0.0f;
        val.color.float32[2] = 0.0f;
        val.color.float32[3] = 0.0f;
    }

    std::uint32_t attachmentsCount = pass->colorAttachmentsCount_;

    if (pass->depthStencilAttachmentInfo_.usage_ == VKW::RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL) {
        clearValues[colorAttachmentsCount].depthStencil.depth = 1.0f;
        clearValues[colorAttachmentsCount].depthStencil.stencil = 0;
        ++attachmentsCount;
    }


    VkRenderPassBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = pass->handle_;
    beginInfo.framebuffer = framebuffer->handle_;
    beginInfo.renderArea.offset.x = 0;
    beginInfo.renderArea.offset.y = 0;
    beginInfo.renderArea.extent.width = width_;
    beginInfo.renderArea.extent.height = height_;
    beginInfo.clearValueCount = attachmentsCount;
    beginInfo.pClearValues = clearValues;


    table_->vkCmdBeginRenderPass(commandReciever->commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
}

void GraphicsPass::Apply(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    std::uint32_t const materialsCount = static_cast<std::uint32_t>(materialDelegatedData_.size());
    for (std::uint32_t i = 0u; i < materialsCount; ++i) {
        Pipeline& pipeline = root_->FindPipeline(materialDelegatedData_[i].pipelineKey_);
        if (pipeline.properties_.pipelineDynamicStateFlags_ & VKW::PIPELINE_DYNAMIC_STATE_VIEWPORT) {
            VkViewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(width_);
            viewport.height = static_cast<float>(height_);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            table_->vkCmdSetViewport(commandReciever->commandBuffer_, 0, 1, &viewport);
        }

        VKW::Pipeline* vkwPipeline = pipelineFactory_->GetPipeline(pipeline.pipelineHandle_);
        VKW::PipelineLayout const* vkwPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwPipeline->layoutHandle);
        VkPipelineLayout const vkPipelineLayout = vkwPipelineLayout->handle_;

        VkCommandBuffer const commandBuffer = commandReciever->commandBuffer_;
        table_->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkwPipeline->vkPipeline_);
        
        Material& material = root_->FindMaterial(materialDelegatedData_[i].materialKey_);
        Material::PerPassData& perPassData = material.perPassData_[i];
        std::uint32_t const materialBindsCount = pipeline.staticLayoutMembersCount_;
        assert(materialBindsCount == perPassData.descritorSetsOwner_.slotsCount_ && "Something has gone wrong, incorrect data.");

        VkDescriptorSet vkSetsToBind[Render::SCOPE_MAX_SETS_MEDIAN];

        for (std::uint32_t j = 0; j < materialBindsCount; ++j) {
            // hmmmmmm, how to choose between per-pass data?
            // The solution can be the following algorythm:
            // 1)   to create all materials before passes;
            // 2)   create passes and form rendergraph;
            // 3)   register Materials in the rendergraph (provided needed data for each renderpass)

            DescriptorSet& set = perPassData.descritorSetsOwner_.slots_[j].descriptorSet_;
            VKW::DescriptorSet* vkwSet = root_->ResourceProxy()->GetDescriptorSet(set.proxyDescriptorSetHandle_, contextId);
            vkSetsToBind[j] = vkwSet->handle_;

            
        }

        if (materialBindsCount > 0)
            table_->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, materialBindsCount, vkSetsToBind, 0, nullptr);


        auto& renderItems = pipeline.renderItems_;

        std::uint32_t const renderItemsCount = static_cast<std::uint32_t>(renderItems.size());
        for (std::uint32_t j = 0u; j < renderItemsCount; ++j) {
            RenderWorkItem& renderItem = renderItems[j];
            std::uint32_t const renderItemSetsCount = renderItem.descriptorSetsOwner_.slotsCount_;
            for (std::uint32_t k = 0; k < renderItemSetsCount; ++k) {
                VKW::DescriptorSet* vkwSet = resourceProxy_->GetDescriptorSet(renderItem.descriptorSetsOwner_.slots_[k].descriptorSet_.proxyDescriptorSetHandle_, contextId);
                vkSetsToBind[k] = vkwSet->handle_;
            }
            //                                                                                              //first descriptor
            table_->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, materialBindsCount, renderItemSetsCount, vkSetsToBind, 0, nullptr);

            assert(renderItem.vertexCount_ >= 0 && "GraphicsPass: renderItem.vertexCount < 0");
            assert(renderItem.indexCount_ >= 0 && "GraphicsPass: renderItem.indexCount < 0");
            if (renderItem.vertexCount_ > 0) {
                VKW::BufferView* vertexBufferView = root_->FindGlobalBuffer(renderItem.vertexBufferKey_, contextId);
                VKW::BufferResource* vertexBuffer = vertexBufferView->providedBuffer_->bufferResource_;

                VkBuffer vkBuffer = vertexBuffer->handle_;
                VkDeviceSize offset = vertexBufferView->offset_ + renderItem.vertexBindOffset_;
                table_->vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkBuffer, &offset);
            }

            if (renderItem.indexCount_ > 0) {
                VKW::BufferView* indexBufferView = root_->FindGlobalBuffer(renderItem.indexBufferKey_, contextId);
                VKW::BufferResource* indexBuffer = indexBufferView->providedBuffer_->bufferResource_;

                VkBuffer vkBuffer = indexBuffer->handle_;
                VkDeviceSize offset = indexBufferView->offset_;
                table_->vkCmdBindIndexBuffer(commandBuffer, vkBuffer, offset + renderItem.indexBindOffset_, VK_INDEX_TYPE_UINT32);
            }
            
            
            if (renderItem.indexCount_ > 0) {
                table_->vkCmdDrawIndexed(commandBuffer, renderItem.indexCount_, 1, 0, 0, 0);
            }
            else if (renderItem.vertexCount_ > 0) {
                table_->vkCmdDraw(commandBuffer, renderItem.vertexCount_, 1, 0, 0);
            }
            
        }
        
    }
}

void GraphicsPass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    table_->vkCmdEndRenderPass(commandReciever->commandBuffer_);
}



ComputePass::ComputePass()
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
{

}

ComputePass::ComputePass(ComputePassDesc const& desc)
    : root_{ desc.root_ }
    , table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourceProxy_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
{

}

ComputePass::ComputePass(ComputePass&& rhs)
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
{
    operator=(std::move(rhs));
}

ComputePass& ComputePass::operator=(ComputePass&& rhs)
{
    std::swap(root_, rhs.root_);
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(pipelineFactory_, rhs.pipelineFactory_);
    std::swap(descriptorLayoutController_, rhs.descriptorLayoutController_);

    return *this;
}

ComputePass::~ComputePass()
{

}

void ComputePass::Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    std::uint32_t constexpr MAX_BARRIERS = 8;
    
    std::uint32_t bufferBarriersCount = 0;
    VkBufferMemoryBarrier bufferBarriers[MAX_BARRIERS];

    std::uint32_t imageBarriersCount = 0;
    VkImageMemoryBarrier imageBarriers[MAX_BARRIERS];
    
    //struct VkBufferMemoryBarrier {
    //    VkStructureType    sType;
    //    const void*        pNext;
    //    VkAccessFlags      srcAccessMask;
    //    VkAccessFlags      dstAccessMask;
    //    uint32_t           srcQueueFamilyIndex;
    //    uint32_t           dstQueueFamilyIndex;
    //    VkBuffer           buffer;
    //    VkDeviceSize       offset;
    //    VkDeviceSize       size;
    //}
    //struct VkImageMemoryBarrier {
    //    VkStructureType            sType;
    //    const void*                pNext;
    //    VkAccessFlags              srcAccessMask;
    //    VkAccessFlags              dstAccessMask;
    //    VkImageLayout              oldLayout;
    //    VkImageLayout              newLayout;
    //    uint32_t                   srcQueueFamilyIndex;
    //    uint32_t                   dstQueueFamilyIndex;
    //    VkImage                    image;
    //    VkImageSubresourceRange    subresourceRange;
    //} VkImageMemoryBarrier;

    std::uint32_t const usagesCount = static_cast<std::uint32_t>(resourceUsageInfos_.size());
    for (std::uint32_t i = 0; i < usagesCount; ++i) {
        ResourceUsageData& usageData = resourceUsageInfos_[i];
        switch (usageData.usage_) {
        case COMPUTE_PASS_RESOURCE_USAGE_BUFFER_READ:
            // just chillin' reading. no need to sync
            break;
        case COMPUTE_PASS_RESOURCE_USAGE_BUFFER_WRITE:
            // if we're just writing, we don't need any memory barrier, we don't care about the content
            break;
        case COMPUTE_PASS_RESOURCE_USAGE_BUFFER_READ_AFTER_WRITE:
        {
            VKW::BufferView* view = root_->FindGlobalBuffer(usageData.resourceKey_, contextId);
            VKW::BufferResource* resource = view->providedBuffer_->bufferResource_;

            auto& barrier = bufferBarriers[bufferBarriersCount++];
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = resource->handle_;
            barrier.offset = view->offset_;
            barrier.size = view->size_;
        }
            break;
        case COMPUTE_PASS_RESOURCE_USAGE_IMAGE_READ:
            break;
        case COMPUTE_PASS_RESOURCE_USAGE_IMAGE_WRITE:
            break;
        case COMPUTE_PASS_RESOURCE_USAGE_IMAGE_READ_AFTER_WRITE:
            break;
        }
    }
}

void ComputePass::Apply(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

void ComputePass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

void ComputePass::RegisterMaterialData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey)
{
    materialDelegatedData_.emplace_back(materialKey, materialPerPassDataId, pipelineKey);
}

void ComputePass::RegisterResourceUsage(ResourceKey const& resourceKey, ComputePassResourceUsage usage)
{
    resourceUsageInfos_.emplace_back(resourceKey, usage);
}

}