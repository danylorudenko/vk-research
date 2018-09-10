#pragma once

#include <vulkan/vulkan.h>
#include <limits>

namespace VKW
{

struct Pipeline
{
    VkPipeline pipeline_;
};

struct PipelineHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}