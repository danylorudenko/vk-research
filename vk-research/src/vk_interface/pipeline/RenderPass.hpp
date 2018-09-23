#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "../runtime/Framebuffer.hpp"

namespace VKW
{

struct RenderPassHandle
{
    std::uint32_t handle_;
};

struct RenderPass
{
    RenderPass(VkRenderPass renderpass, FramebufferHandle framebuffer);

    VkRenderPass renderPass_;
    FramebufferHandle framebuffer_;
};

}