#pragma once

#include <vulkan\vulkan.h>
#include <cstdint>

#include <vk_interface\pipeline\DescriptorLayout.hpp>

namespace VKW
{

struct Pipeline
{
    static std::uint32_t constexpr MAX_SHADER_STAGES = 6;
    static std::uint32_t constexpr MAX_VERTEX_ATTRIBUTES = 6;
    static std::uint32_t constexpr MAX_VIEWPORTS = 4;
    
    VkPipeline vkPipeline_;
    PipelineLayout* layout_;
};

}