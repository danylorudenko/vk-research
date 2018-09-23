#include "RenderPassFactory.hpp"
#include "../Tools.hpp"
#include "../resources/ResourcesController.hpp"
#include "../ImportTable.hpp"
#include "../Device.hpp"

namespace VKW
{

RenderPassFactory::RenderPassFactory()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{}

RenderPassFactory::RenderPassFactory(RenderPassFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ nullptr }
{}

RenderPassFactory::RenderPassFactory(RenderPassFactory&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

RenderPassFactory& RenderPassFactory::operator=(RenderPassFactory&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(renderPasses_, rhs.renderPasses_);

    return *this;
}

RenderPassFactory::~RenderPassFactory()
{}

RenderPassHandle RenderPassFactory::AssembleRenderPass(RenderPassDesc const& desc)
{
    std::vector<VkAttachmentDescription> attachments;

    std::vector<VkAttachmentReference> colorAttachments;
    attachments.resize(desc.colorAttachmentsCount_);
    for (auto i = 0u; i < desc.colorAttachmentsCount_; ++i) {
        ImageResource* imageRes = resourcesController_->GetImage(desc.colorAttachments_[i]);
        auto& attachment = attachments[i];
        attachment.format = imageRes->format_;
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
        colorReference.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkAttachmentReference depthStencilReference = { desc.colorAttachmentsCount_, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription spInfo;
    spInfo.inputAttachmentCount = 0;
    spInfo.pInputAttachments = nullptr;
    spInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spInfo.preserveAttachmentCount = 0;
    spInfo.pPreserveAttachments = nullptr;
    spInfo.pResolveAttachments = nullptr;
    spInfo.colorAttachmentCount = desc.colorAttachmentsCount_;
    spInfo.pColorAttachments = colorAttachments.data();
    spInfo.pDepthStencilAttachment = &depthStencilReference;
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
    
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateRenderPass(device_->Handle(), &rpInfo, nullptr, &renderPass));

    // create framebuffer and return value

    return RenderPassHandle{};
}

}