#include "FramebufferController.hpp"

#include <utility>
#include "../ImportTable.hpp"
#include "../Device.hpp"
#include "../resources/ResourcesController.hpp"
#include "../pipeline/RenderPassController.hpp"
#include "../pipeline/RenderPass.hpp"

namespace VKW
{

FramebufferController::FramebufferController()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
    , renderPassController_{ nullptr }
{

}

FramebufferController::FramebufferController(FramebufferFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ desc.resourcesController_ }
    , renderPassController_{ desc.renderPassController_ }
{

}

FramebufferController::FramebufferController(FramebufferController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
    , renderPassController_{ nullptr }
{
    operator=(std::move(rhs));
}

FramebufferController& FramebufferController::operator=(FramebufferController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(renderPassController_, rhs.renderPassController_);

    return *this;
}

FramebufferHandle FramebufferController::CreateFramebuffer(FramebufferDesc const& desc)
{
    RenderPass const* renderPass = renderPassController_->GetRenderPass(desc.renderPass_);

    VkRenderPass const vkRenderPass = renderPass->handle_;
    auto const attachmentCount = renderPass->colorAttachmentsCount_ + renderPass->depthStencilAttachmentInfo_.used_ ? 1 : 0;
    VkImageView* attachments = nullptr; // TODO

    VkFramebufferCreateInfo fbInfo;
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = nullptr;
    fbInfo.flags = VK_FLAGS_NONE;
    fbInfo.renderPass = vkRenderPass;
    fbInfo.attachmentCount = attachmentCount;
    fbInfo.pAttachments = attachments;
    fbInfo.width = desc.width_;
    fbInfo.height = desc.height_;
    fbInfo.layers = 1;

    VkFramebuffer result = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateFramebuffer(device_->Handle(), &fbInfo, nullptr, &result));
    
}

}