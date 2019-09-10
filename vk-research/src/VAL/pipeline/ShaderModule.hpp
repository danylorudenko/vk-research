#pragma once

#include <vulkan\vulkan.h>
#include <limits>
#include <cstdint>
#include <string>

namespace VAL
{

enum ShaderModuleType
{
    SHADER_MODULE_TYPE_COMPUTE,
    SHADER_MODULE_TYPE_VERTEX,
    SHADER_MODULE_TYPE_FRAGMENT
};

struct ShaderModule
{
    ShaderModule(VkShaderModule module);

    VkShaderModule handle_;
    ShaderModuleType type_;
    std::string entryPoint_;
};

struct ShaderModuleHandle
{
    ShaderModule* handle_ = nullptr;
};

}