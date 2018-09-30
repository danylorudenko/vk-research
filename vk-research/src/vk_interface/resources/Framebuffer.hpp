#pragma once

#include <cstdint>
#include <limits>
#include <vulkan/vulkan.h>

namespace VKW
{

struct FramebufferHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct Framebuffer
{
    VkFramebuffer handle_ = VK_NULL_HANDLE;
};

}