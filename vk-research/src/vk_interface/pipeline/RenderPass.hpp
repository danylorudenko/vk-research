#pragma once

#include <cstdint>
#include <vulkan\vulkan.h>

#include <vk_interface\resources\Resource.hpp>

namespace VKW
{


enum RenderPassAttachmentUsage
{
    RENDER_PASS_ATTACHMENT_USAGE_NONE,
    RENDER_PASS_ATTACHMENT_USAGE_COLOR_PRESERVE,
    RENDER_PASS_ATTACHMENT_USAGE_COLOR_CLEAR,
    RENDER_PASS_ATTACHMENT_USAGE_COLOR_CLEAR_PRESENT,
    RENDER_PASS_ATTACHMENT_USAGE_COLOR_PRESERVE_PRESENT,
    RENDER_PASS_ATTACHMENT_USAGE_DEPTH_STENCIL
};

struct RenderPassHandle
{
    std::uint32_t id_;
};

struct RenderPassAttachmentInfo
{
    VkFormat format_;
    RenderPassAttachmentUsage usage_;
};

struct RenderPass
{
    static std::uint32_t constexpr MAX_COLOR_ATTACHMENTS = 4;
    static std::uint32_t constexpr MAX_ATTACHMENTS = MAX_COLOR_ATTACHMENTS + 1;
    
    RenderPass(VkRenderPass renderpass, std::uint32_t colorAttachmentsCount, RenderPassAttachmentInfo* colorAttachmentsinfo, RenderPassAttachmentInfo const& depthStencilInfo);

    VkRenderPass handle_;
    std::uint32_t colorAttachmentsCount_;
    RenderPassAttachmentInfo colorAttachmentsInfo_[MAX_COLOR_ATTACHMENTS];
    RenderPassAttachmentInfo depthStencilAttachmentInfo_;
    
};

}