#pragma once

#include <cstdint>
#include <limits>
#include <vulkan\vulkan.h>

#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\pipeline\RenderPass.hpp>

namespace VKW
{

struct Framebuffer
{
    Framebuffer() = default;

    ~Framebuffer() = default;

    VkFramebuffer handle_ = VK_NULL_HANDLE;

    std::uint32_t width_;
    std::uint32_t height_;

    std::uint32_t colorAttachmentsCount_;
    ImageView* colorAttachments_[RenderPass::MAX_COLOR_ATTACHMENTS];
    ImageView* depthStencilAttachment_;
};

struct FramebufferHandle
{
    Framebuffer* framebuffer_;
};


}