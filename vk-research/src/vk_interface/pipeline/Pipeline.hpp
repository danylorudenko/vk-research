#pragma once

#include <vulkan\vulkan.h>
#include <limits>

namespace VKW
{

struct Pipeline
{
    VkPipeline pipeline_;
};

struct PipelineHandle
{
    Pipeline* pipeline_;
};

}