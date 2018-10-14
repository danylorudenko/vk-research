#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "../resources/Resource.hpp"

namespace VKW
{

struct RenderPassHandle
{
    std::uint32_t id_;
};

struct RenderPassAttachmentInfo
{
    VkFormat format_;

    bool used_ = false;
};

struct RenderPass
{
    RenderPass(VkRenderPass renderpass, std::uint32_t colorAttachmentsCount, RenderPassAttachmentInfo* colorAttachmentsinfo, RenderPassAttachmentInfo const& depthStencilInfo);

    VkRenderPass handle_;
    std::uint32_t colorAttachmentsCount_;
    RenderPassAttachmentInfo colorAttachmentsInfo_[6];
    RenderPassAttachmentInfo depthStencilAttachmentInfo_;
    
};

}