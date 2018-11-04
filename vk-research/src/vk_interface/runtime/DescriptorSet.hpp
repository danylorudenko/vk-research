#pragma once

#include "../pipeline/DescriptorLayout.hpp"

#include <vulkan/vulkan.h>

namespace VKW
{

struct DescriptorSet
{
    VkDescriptorSet handle_;
    DescriptorSetLayoutHandle layout_;
};

struct DescriptorSetHandle
{
    DescriptorSet* handle_ = nullptr;
};

}