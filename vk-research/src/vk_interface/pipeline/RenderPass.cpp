#include "RenderPass.hpp"

namespace VKW
{

RenderPass::RenderPass(VkRenderPass renderpass, FramebufferHandle framebuffer)
    : renderPass_{ renderpass }
    , framebuffer_{ framebuffer }
{

}

}