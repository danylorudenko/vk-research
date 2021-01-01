#pragma once

#include <vk_interface\pipeline\DescriptorLayout.hpp>

#include <vulkan\vulkan.h>

namespace VKW
{

struct DescriptorSet
{
    VkDescriptorSet handle_;
    DescriptorSetLayout const* layout_;
};

struct DescriptorSetHandle
{
    DescriptorSet* handle_ = nullptr;
};

}