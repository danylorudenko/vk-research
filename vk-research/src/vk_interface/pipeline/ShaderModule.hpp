#pragma once

#include <vulkan/vulkan.h>
#include <limits>
#include <cstdint>

namespace VKW
{

struct ShaderModule
{
    VkShaderModule handle_;
};

struct ShaderModuleHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}