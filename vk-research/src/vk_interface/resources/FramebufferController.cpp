#include "FramebufferController.hpp"

#include <utility>
#include "..\ImportTable.hpp"
#include "..\Device.hpp"
#include "..\image\ImagesProvider.hpp"
#include "..\pipeline\RenderPassController.hpp"
#include "..\pipeline\RenderPass.hpp"

namespace VKW
{

FramebufferController::FramebufferController()
    : table_{ nullptr }
    , device_{ nullptr }
    , imagesProvider_{ nullptr }
    , renderPassController_{ nullptr }
{

}

FramebufferController::FramebufferController(FramebufferControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , renderPassController_{ desc.renderPassController_ }
{

}

FramebufferController::FramebufferController(FramebufferController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , imagesProvider_{ nullptr }
    , renderPassController_{ nullptr }
{
    operator=(std::move(rhs));
}

FramebufferController& FramebufferController::operator=(FramebufferController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(framebuffers_, rhs.framebuffers_);

    return *this;
}

FramebufferController::~FramebufferController()
{
    VkDevice const device = device_->Handle();
    for (auto& framebuffer : framebuffers_) {
        table_->vkDestroyFramebuffer(device, framebuffer->handle_, nullptr);
        delete framebuffer;
    }
}

Framebuffer* FramebufferController::GetFramebuffer(FramebufferHandle handle)
{
    return handle.framebuffer_;
}

FramebufferHandle FramebufferController::CreateFramebuffer(FramebufferDesc const& desc)
{
    RenderPass const* renderPass = renderPassController_->GetRenderPass(desc.renderPass_);

    bool const renderPassUsesDepthStencil = renderPass->depthStencilAttachmentInfo_.usage_ == RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL;

    std::uint32_t const colorAttachmentsCount = renderPass->colorAttachmentsCount_;
    std::uint32_t const allAttachmentsCount = renderPass->colorAttachmentsCount_ + (renderPassUsesDepthStencil ? 1 : 0);

    VkImageView attachments[RenderPass::MAX_ATTACHMENTS];

    for (auto i = 0u; i < colorAttachmentsCount; ++i) {
        ImageView* imageView = imagesProvider_->GetImageView(desc.colorAttachments[i]);
        attachments[i] = imageView->handle_;
    }

    if (renderPassUsesDepthStencil) {
        assert(desc.depthStencilAttachment != nullptr && "RenderPass uses depthstencil attachment but no image was provided for framebuffer");
        ImageView* imageView = imagesProvider_->GetImageView(*desc.depthStencilAttachment);
        attachments[colorAttachmentsCount] = imageView->handle_;
    }

    VkFramebufferCreateInfo fbInfo;
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = nullptr;
    fbInfo.flags = VK_FLAGS_NONE;
    fbInfo.renderPass = renderPass->handle_;
    fbInfo.attachmentCount = allAttachmentsCount;
    fbInfo.pAttachments = attachments;
    fbInfo.width = desc.width_;
    fbInfo.height = desc.height_;
    fbInfo.layers = 1;

    VkFramebuffer vkFramebuffer = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateFramebuffer(device_->Handle(), &fbInfo, nullptr, &vkFramebuffer));

    Framebuffer* framebuffer = new Framebuffer{};
    framebuffer->handle_ = vkFramebuffer;
    framebuffer->width_ = desc.width_;
    framebuffer->height_ = desc.height_;
    framebuffer->depthStencilAttachment_ = renderPassUsesDepthStencil ? *desc.depthStencilAttachment : ImageViewHandle{};
    framebuffer->colorAttachmentsCount_ = renderPass->colorAttachmentsCount_;
    for (auto i = 0u; i < colorAttachmentsCount; ++i) {
        framebuffer->colorAttachments_[i] = desc.colorAttachments[i];
    }

    framebuffers_.emplace_back(framebuffer);

    return FramebufferHandle{ framebuffer };
}

}