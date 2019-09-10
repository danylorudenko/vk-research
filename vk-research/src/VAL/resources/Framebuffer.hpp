#pragma once

#include <cstdint>
#include <limits>
#include <vulkan\vulkan.h>

#include "..\image\ImageView.hpp"
#include "..\pipeline\RenderPass.hpp"

namespace VAL
{

struct Framebuffer
{
    VkFramebuffer handle_ = VK_NULL_HANDLE;

    std::uint32_t width_;
    std::uint32_t height_;

    std::uint32_t colorAttachmentsCount_;
    ImageViewHandle colorAttachments_[RenderPass::MAX_COLOR_ATTACHMENTS];
    ImageViewHandle depthStencilAttachment_;
};

struct FramebufferHandle
{
    Framebuffer* framebuffer_;
};


}