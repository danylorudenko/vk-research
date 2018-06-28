#pragma once

#include <vulkan/vulkan.h>

#include "..\class_features\NonCopyable.hpp"

#include "VulkanImportTable.hpp"
#include "Instance.hpp"

namespace VKW
{

class Device
    : public NonCopyable
{
public:
    Device();
    Device(VulkanImportTable* table, Instance& instance, std::vector<std::string> const& requiredExtensions);

    Device(Device&& rhs);
    Device& operator=(Device&& rhs);

    operator bool() const;

    VkDevice Handle() const;

private:
    VkDevice device_;
    VulkanImportTable* table_;
};

}