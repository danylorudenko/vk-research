#pragma once

#include <vulkan\vulkan.h>

#include "..\system\DynamicLibrary.hpp"

namespace VKW
{

class VulkanImportTable
{
public:
    VulkanImportTable();
    VulkanImportTable(DynamicLibrary& vulkanLibrary);

public:
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;

    PFN_vkCreateInstance vkCreateInstance = nullptr;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;

    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    
    PFN_vkCreateDevice vkCreateDevice = nullptr;
};

}