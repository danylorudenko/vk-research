#pragma once

#include <vulkan\vulkan.h>

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

class VulkanImportTable
    : public NonCopyable
{
public:
    VulkanImportTable();
    VulkanImportTable(DynamicLibrary& vulkanLibrary);

    VulkanImportTable(VulkanImportTable&& rhs);
    VulkanImportTable& operator=(VulkanImportTable&& rhs);

    ~VulkanImportTable();

public:
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
};