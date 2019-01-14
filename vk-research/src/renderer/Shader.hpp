#pragma once

#include "..\vk_interface\pipeline\ShaderModule.hpp"

class Shader
{
public:
    Shader(VKW::ShaderModuleHandle shaderModule);

    Shader(Shader const& rhs) = delete;
    Shader& operator=(Shader& rhs) = delete;

    Shader(Shader&& rhs);
    Shader& operator=(Shader&& rhs);

    ~Shader();

private:
    VKW::ShaderModuleHandle shaderModule_;

};