#include "RenderPass.hpp"

namespace VKW
{

RenderPass::RenderPass(VkRenderPass renderpass, std::uint32_t colorAttachmentsCount, RenderPassAttachmentInfo* colorAttachmentsinfo, RenderPassAttachmentInfo const& depthStencilInfo)
    : handle_{ renderpass }
    , colorAttachmentsCount_{ colorAttachmentsCount }
    , depthStencilAttachmentInfo_{ depthStencilInfo }
{
    for (auto i = 0u; i < colorAttachmentsCount_; ++i) {
        colorAttachmentsInfo_[i] = colorAttachmentsinfo[i];
    }
}

}