#pragma once

#include <vulkan\vulkan.h>
#include <limits>
#include <cstdint>

namespace VKW
{

struct ShaderModule
{
    ShaderModule(VkShaderModule module);

    VkShaderModule handle_;
};

struct ShaderModuleHandle
{
    ShaderModule* handle_ = nullptr;
};

}