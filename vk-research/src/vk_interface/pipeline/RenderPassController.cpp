#include "RenderPassController.hpp"
#include "../Tools.hpp"
#include "../resources/ResourcesController.hpp"
#include "../ImportTable.hpp"
#include "../Device.hpp"

namespace VKW
{

RenderPassController::RenderPassController()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{}

RenderPassController::RenderPassController(RenderPassFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ nullptr }
{}

RenderPassController::RenderPassController(RenderPassController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

RenderPassController& RenderPassController::operator=(RenderPassController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(renderPasses_, rhs.renderPasses_);

    return *this;
}

RenderPassHandle RenderPassController::AssembleRenderPass(RenderPassDesc const& desc)
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    RenderPassAttachmentInfo colorAttachmentsInfo[6];
    RenderPassAttachmentInfo depthStencilAttachmentInfo;

    assert(desc.colorAttachmentsCount_ <= 6 && "Maximum number color attachments supported is 6.");

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachments;

    attachments.resize(desc.colorAttachmentsCount_ + (desc.depthStencilAttachment_ != nullptr ? 1 : 0));
    colorAttachments.resize(desc.colorAttachmentsCount_);

    for (auto i = 0u; i < desc.colorAttachmentsCount_; ++i) {

        colorAttachmentsInfo[i].format_ = desc.colorAttachments_[i].format_;
        colorAttachmentsInfo[i].used_ = true;

        auto& attachment = attachments[i];
        attachment.format = desc.colorAttachments_[i].format_;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.flags = VK_FLAGS_NONE;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;

        auto& colorReference = colorAttachments[i];
        colorReference.attachment = i;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthStencilReference;
    if (desc.depthStencilAttachment_) {
        depthStencilAttachmentInfo.format_ = desc.depthStencilAttachment_->format_;
        depthStencilAttachmentInfo.used_ = true;
        depthStencilReference = { desc.colorAttachmentsCount_, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    }

    VkSubpassDescription spInfo;
    spInfo.inputAttachmentCount = 0;
    spInfo.pInputAttachments = nullptr;
    spInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spInfo.preserveAttachmentCount = 0;
    spInfo.pPreserveAttachments = nullptr;
    spInfo.pResolveAttachments = nullptr;
    spInfo.colorAttachmentCount = desc.colorAttachmentsCount_;
    spInfo.pColorAttachments = colorAttachments.data();
    spInfo.pDepthStencilAttachment = desc.depthStencilAttachment_ ? &depthStencilReference : nullptr;
    spInfo.flags = VK_FLAGS_NONE;

    
    VkRenderPassCreateInfo rpInfo;
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext = nullptr;
    rpInfo.dependencyCount = 0;
    rpInfo.pDependencies = nullptr;
    rpInfo.flags = VK_FLAGS_NONE;
    rpInfo.attachmentCount = desc.colorAttachmentsCount_;
    rpInfo.pAttachments = attachments.data();
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &spInfo;
    
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