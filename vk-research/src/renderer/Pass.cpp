#include "Pass.hpp"
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\pipeline\PipelineFactory.hpp"
#include "..\vk_interface\pipeline\Pipeline.hpp"
#include "..\vk_interface\ImportTable.hpp"
#include "..\vk_interface\Device.hpp"
#include "..\vk_interface\worker\Worker.hpp"
#include "..\vk_interface\ImportTable.hpp"
#include "..\vk_interface\Device.hpp"

#include <utility>

namespace Render
{

Pass::Pass()
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

Pass::Pass(PassDesc const& desc)
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

        VKW::ImageViewHandle imageViewHandle = desc.framedDescriptorsHub_->contexts_[0].imageViews_[desc.colorAttachments_[i].id_];
        VKW::ImageView* imageView = desc.imagesProvider_->GetImageView(imageViewHandle);
        colorAttachmentDescs[i].format_ = imageView->format_;
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
        framebufferProxyDesc.colorAttachments_[j] = desc.colorAttachments_[j];
    }

    framebufferProxyDesc.depthStencilAttachment_ = desc.depthStencilAttachment_;
    
    framebuffer_ = resourceProxy_->CreateFramebuffer(framebufferProxyDesc);
}

Pass::Pass(Pass&& rhs)
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

Pass& Pass::operator=(Pass&& rhs)
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

Pass::~Pass()
{
    
}

VKW::RenderPassHandle Pass::VKWRenderPass() const
{
    return vkRenderPass_;
}

void Pass::RegisterMaterialData(Material::PerPassData* perPassData, PipelineKey const& pipelineKey)
{
    materialDelegatedData_.emplace_back(perPassData, pipelineKey);
}

void Pass::Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
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
        val.color.float32[3] = 1.0f;
    }

    if (pass->depthStencilAttachmentInfo_.usage_ == VKW::RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL) {
        clearValues[colorAttachmentsCount].depthStencil.depth = 0.0f;
        clearValues[colorAttachmentsCount].depthStencil.stencil = 0;
    }

    std::uint32_t const attachmentsCount = pass->colorAttachmentsCount_ + pass->depthStencilAttachmentInfo_.usage_ == VKW::RENDER_PASS_ATTACHMENT_USAGE_NONE ? 0 : 1;

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

void Pass::Render(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    std::uint32_t const materialsCount = static_cast<std::uint32_t>(materialDelegatedData_.size());
    for (std::uint32_t i = 0u; i < materialsCount; ++i) {
        Pipeline& pipeline = root_->FindPipeline(materialDelegatedData_[i].pipelineKey_);
        VKW::Pipeline* vkwPipeline = pipelineFactory_->GetPipeline(pipeline.pipelineHandle_);
        VKW::PipelineLayout const* vkwPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwPipeline->layoutHandle);
        VkPipelineLayout const vkPipelineLayout = vkwPipelineLayout->handle_;

        VkCommandBuffer const commandBuffer = commandReciever->commandBuffer_;
        table_->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkwPipeline->vkPipeline_);
        

        Material::PerPassData* perPassData = materialDelegatedData_[i].materialPerPassData_;
        std::uint32_t const materialBindsCount = pipeline.staticLayoutMembersCount_;
        assert(materialBindsCount == perPassData->descritorSetsOwner_.slotsCount_ && "Something has gone wrong, incorrect data.");

        VkDescriptorSet vkSetsToBind[Render::SCOPE_MAX_SETS_MEDIAN];

        for (std::uint32_t j = 0; j < materialBindsCount; ++j) {
            // hmmmmmm, how to choose between per-pass data?
            // The solution can be the following algorythm:
            // 1)   to create all materials before passes;
            // 2)   create passes and form rendergraph;
            // 3)   register Materials in the rendergraph (provided needed data for each renderpass)

            DescriptorSet& set = perPassData->descritorSetsOwner_.slots_[j].descriptorSet_;
            VKW::DescriptorSet* vkwSet = root_->ResourceProxy()->GetDescriptorSet(set.proxyDescriptorSetHandle_, contextId);
            vkSetsToBind[j] = vkwSet->handle_;

            
        }

        //typedef void (VKAPI_PTR *PFN_vkCmdBindDescriptorSets)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, 
        //uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
        
        if (materialBindsCount > 0)
            table_->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, materialBindsCount, vkSetsToBind, 0, nullptr);


        // temporaty testing zalepa
        // TODO

        auto& renderItems = pipeline.renderItems_;

        std::uint32_t const renderItemsCount = static_cast<std::uint32_t>(renderItems.size());
        for (std::uint32_t j = 0u; j < renderItemsCount; ++j) {
            RenderWorkItem& renderItem = renderItems[i];
            std::uint32_t const renderItemSetsCount = renderItem.descriptorSetsOwner_.slotsCount_;
            for (std::uint32_t k = 0; k < renderItemsCount; ++k) {
                VKW::DescriptorSet* vkwSet = resourceProxy_->GetDescriptorSet(renderItem.descriptorSetsOwner_.slots_[k].descriptorSet_.proxyDescriptorSetHandle_, contextId);
                vkSetsToBind[k] = vkwSet->handle_;
            }

            table_->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, renderItemSetsCount, vkSetsToBind, 0, nullptr);

            assert(renderItem.vertexCount_ >= 0 && "Pass: renderItem.vertexCount < 0");
            assert(renderItem.indexCount_ >= 0 && "Pass: renderItem.indexCount < 0");
            if (renderItem.vertexCount_ > 0) {
                VKW::BufferView* vertexBufferView = root_->FindGlobalBuffer(renderItem.vertexBufferKey_, contextId);
                VKW::BufferResource* vertexBuffer = resourceProxy_->GetResource(vertexBufferView->providedBuffer_->bufferResource_);

                VkBuffer vkBuffer = vertexBuffer->handle_;
                VkDeviceSize offset = vertexBufferView->offset_ + vertexBuffer->memory_.offset_;
                table_->vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkBuffer, &offset);
            }

            if (renderItem.indexCount_ > 0) {
                VKW::BufferView* indexBufferView = root_->FindGlobalBuffer(renderItem.indexBufferKey_, contextId);
                VKW::BufferResource* indexBuffer = resourceProxy_->GetResource(indexBufferView->providedBuffer_->bufferResource_);

                VkBuffer vkBuffer = indexBuffer->handle_;
                VkDeviceSize offset = indexBufferView->offset_ + indexBuffer->memory_.offset_;
                table_->vkCmdBindIndexBuffer(commandBuffer, vkBuffer, offset, VK_INDEX_TYPE_UINT32);
            }
            
            
            if (renderItem.indexCount_ > 0) {
                table_->vkCmdDrawIndexed(commandBuffer, renderItem.indexCount_, 1, 0, 0, 0);
            }
            else {
                table_->vkCmdDraw(commandBuffer, renderItem.vertexCount_, 1, 0, 0);
            }
            
        }
        
    }
}

void Pass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    table_->vkCmdEndRenderPass(commandReciever->commandBuffer_);
}

}