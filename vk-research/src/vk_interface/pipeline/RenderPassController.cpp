#include "RenderPassController.hpp"
#include "..\Tools.hpp"
#include "..\resources\ResourcesController.hpp"
#include "..\ImportTable.hpp"
#include "..\Device.hpp"

namespace VKW
{

RenderPassController::RenderPassController()
    : table_{ nullptr }
    , device_{ nullptr }
{}

RenderPassController::RenderPassController(RenderPassControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{}

RenderPassController::RenderPassController(RenderPassController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

RenderPassController& RenderPassController::operator=(RenderPassController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(renderPasses_, rhs.renderPasses_);

    return *this;
}

RenderPassHandle RenderPassController::AssembleRenderPass(RenderPassDesc const& desc)
{
    static RenderPassAttachmentInfo colorAttachmentsInfo[RenderPass::MAX_COLOR_ATTACHMENTS];
    RenderPassAttachmentInfo depthStencilAttachmentInfo;

    assert(desc.colorAttachmentsCount_ <= RenderPass::MAX_COLOR_ATTACHMENTS && "Maximum number color attachments supported is RenderPass::MAX_COLOR_ATTACHMENTS.");

    static VkAttachmentDescription attachmentDescriptions[RenderPass::MAX_ATTACHMENTS];
    static VkAttachmentReference colorAttachmentReferences[RenderPass::MAX_COLOR_ATTACHMENTS];

    std::uint32_t const allAttachmentsCount = desc.colorAttachmentsCount_ + (desc.depthStencilAttachment_ != nullptr ? 1 : 0);

    for (auto i = 0u; i < desc.colorAttachmentsCount_; ++i) {

        colorAttachmentsInfo[i].format_ = desc.colorAttachments_[i].format_;
        colorAttachmentsInfo[i].usage_ = RENDER_PASS_ATTACHMENT_USAGE_COLOR;

        auto& attachment = attachmentDescriptions[i];
        attachment.format = desc.colorAttachments_[i].format_;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.flags = VK_FLAGS_NONE;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;

        auto& colorReference = colorAttachmentReferences[i];
        colorReference.attachment = i;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthStencilReference;
    if (desc.depthStencilAttachment_) {
        auto& depthStencilAttachment = attachmentDescriptions[desc.colorAttachmentsCount_];
        depthStencilAttachment.format = desc.depthStencilAttachment_->format_;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.flags = VK_FLAGS_NONE;
        depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        depthStencilReference = { desc.colorAttachmentsCount_, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        depthStencilAttachmentInfo.format_ = desc.depthStencilAttachment_->format_;
        depthStencilAttachmentInfo.usage_ = RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL;
    }
    else {
        depthStencilAttachmentInfo.usage_ = RENDER_PASS_ATTACHMENT_USAGE_NONE;
    }

    VkSubpassDescription spInfo;
    spInfo.inputAttachmentCount = 0;
    spInfo.pInputAttachments = nullptr;
    spInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spInfo.preserveAttachmentCount = 0;
    spInfo.pPreserveAttachments = nullptr;
    spInfo.pResolveAttachments = nullptr;
    spInfo.colorAttachmentCount = desc.colorAttachmentsCount_;
    spInfo.pColorAttachments = colorAttachmentReferences;
    spInfo.pDepthStencilAttachment = desc.depthStencilAttachment_ ? &depthStencilReference : nullptr;
    spInfo.flags = VK_FLAGS_NONE;

    
    VkRenderPassCreateInfo rpInfo;
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext = nullptr;
    rpInfo.dependencyCount = 0;
    rpInfo.pDependencies = nullptr;
    rpInfo.flags = VK_FLAGS_NONE;
    rpInfo.attachmentCount = allAttachmentsCount;
    rpInfo.pAttachments = attachmentDescriptions;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &spInfo;
    
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateRenderPass(device_->Handle(), &rpInfo, nullptr, &renderPass));

    renderPasses_.emplace_back(renderPass, desc.colorAttachmentsCount_, colorAttachmentsInfo, depthStencilAttachmentInfo);

    return RenderPassHandle{ static_cast<std::uint32_t>(renderPasses_.size()) - 1 };
}

void RenderPassController::FreeRenderPass(RenderPassHandle handle)
{
    auto& renderPass = renderPasses_[handle.id_];
    table_->vkDestroyRenderPass(device_->Handle(), renderPass.handle_, nullptr);
    renderPasses_.erase(renderPasses_.begin() + handle.id_);
}

RenderPass* RenderPassController::GetRenderPass(RenderPassHandle handle)
{
    return &renderPasses_[handle.id_];
}

RenderPassController::~RenderPassController()
{
    auto device = device_->Handle();
    for (auto const& pass : renderPasses_) {
        table_->vkDestroyRenderPass(device, pass.handle_, nullptr);
    }
}

}