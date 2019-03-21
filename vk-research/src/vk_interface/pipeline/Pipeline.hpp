#pragma once

#include <vulkan\vulkan.h>
#include <limits>
#include <cstdint>

#include "DescriptorLayout.hpp"

namespace VKW
{

struct Pipeline
{
    static std::uint32_t constexpr MAX_SHADER_STAGES = 6;
    static std::uint32_t constexpr MAX_VERTEX_ATTRIBUTES = 6;
    static std::uint32_t constexpr MAX_VIEWPORTS = 4;
    
    VkPipeline vkPipeline_;
    PipelineLayoutHandle layoutHandle;
};

struct PipelineHandle
{
    Pipeline* pipeline_;
};

}