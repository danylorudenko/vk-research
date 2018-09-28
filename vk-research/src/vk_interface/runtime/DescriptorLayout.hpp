#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <limits>

namespace VKW
{

struct DescriptorSetLayoutHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct DescriptorSetLayout
{
    VkDescriptorSetLayout handle_ = VK_NULL_HANDLE;
};



struct DescriptorPipelineLayoutHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct DescriptorPipelineLayout
{
    VkPipelineLayout handle_ = VK_NULL_HANDLE;
};

}