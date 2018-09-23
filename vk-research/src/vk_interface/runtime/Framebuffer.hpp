#pragma once

#include <cstdint>
#include <limits>
#include <vulkan/vulkan.h>

namespace VKW
{

struct FramebufferHandle
{
    std::uint32_t handle_ = std::numeric_limits<std::uint32_t>::max();
};

struct Framebuffer
{
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
};

}