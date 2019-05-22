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
{
}

RenderPassController::RenderPassController(RenderPassControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
}

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

    static VkAttachmentDescription vkAttachmentDescriptions[RenderPass::MAX_ATTACHMENTS];
    static VkAttachmentReference colorAttachmentReferences[RenderPass::MAX_COLOR_ATTACHMENTS];

    std::uint32_t const allAttachmentsCount = desc.colorAttachmentsCount_ + (desc.depthStencilAttachment_ != nullptr ? 1 : 0);

    for (auto i = 0u; i < desc.colorAttachmentsCount_; ++i) {

        colorAttachmentsInfo[i].format_ = desc.colorAttachments_[i].format_;
        colorAttachmentsInfo[i].usage_ = desc.colorAttachments_[i].usage_;
        assert(colorAttachmentsInfo[i].usage_ != RENDER_PASS_ATTACHMENT_USAGE_NONE && "Attachment usage was not specified!");

        auto& vkAttachment = vkAttachmentDescriptions[i];
        vkAttachment.format = desc.colorAttachments_[i].format_;

        switch (colorAttachmentsInfo[i].usage_) {
        case RENDER_PASS_ATTACHMENT_USAGE_COLOR_CLEAR:
            vkAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            vkAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            vkAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;

        case RENDER_PASS_ATTACHMENT_USAGE_COLOR_CLEAR_PRESENT:
            vkAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            vkAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            vkAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;

        case RENDER_PASS_ATTACHMENT_USAGE_COLOR_PRESERVE_PRESENT:
            vkAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            vkAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            vkAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;

        default:
            assert(false && "RENDER_PASS_ATTACHMENT_USAGE_ not supported for color attachments.");
        }
        vkAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vkAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vkAttachment.flags = VK_FLAGS_NONE;
        vkAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        auto& colorReference = colorAttachmentReferences[i];
        colorReference.attachment = i;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthStencilReference;
    if (desc.depthStencilAttachment_) {
        auto& depthStencilAttachment = vkAttachmentDescriptions[desc.colorAttachmentsCount_];
        depthStencilAttachment.format = desc.depthStencilAttachment_->format_;
        depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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


    VkSubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    
    VkRenderPassCreateInfo rpInfo;
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext = nullptr;
    rpInfo.dependencyCount = 2;
    rpInfo.pDependencies = dependencies;
    rpInfo.flags = VK_FLAGS_NONE;
    rpInfo.attachmentCount = allAttachmentsCount;
    rpInfo.pAttachments = vkAttachmentDescriptions;
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