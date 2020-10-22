#pragma once

#include <vulkan\vulkan.h>
#include <string>

namespace VKW
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

}