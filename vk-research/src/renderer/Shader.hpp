#pragma once

#include <vk_interface\pipeline\ShaderModule.hpp>

namespace Render
{

struct Shader
{
    VKW::ShaderModuleHandle vkwShaderModuleHandle_;
};

}