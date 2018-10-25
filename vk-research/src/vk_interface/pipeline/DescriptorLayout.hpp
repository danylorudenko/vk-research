#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <limits>

namespace VKW
{

struct DescriptorSetLayout
{
    VkDescriptorSetLayout handle_ = VK_NULL_HANDLE;
};

struct DescriptorSetLayoutHandle
{
    DescriptorSetLayout* layout_ = nullptr;
};


struct DescriptorPipelineLayout
{
    VkPipelineLayout handle_ = VK_NULL_HANDLE;
};

struct DescriptorPipelineLayoutHandle
{
    DescriptorPipelineLayout* layout_ = nullptr;
};

}