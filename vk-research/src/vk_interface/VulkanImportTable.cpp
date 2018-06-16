#include "VulkanImportTable.hpp"

namespace VKW
{

VulkanImportTable::VulkanImportTable() = default;

VulkanImportTable::VulkanImportTable(DynamicLibrary& vulkanLibrary)
{
    assert(vulkanLibrary);
    
    vkGetInstanceProcAddr = vulkanLibrary.GetProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vkGetDeviceProcAddr = vulkanLibrary.GetProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

    vkCreateInstance = vulkanLibrary.GetProcAddress<PFN_vkCreateInstance>("vkCreateInstance");
    vkEnumerateInstanceLayerProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    vkEnumerateInstanceExtensionProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");

    vkEnumeratePhysicalDevices = vulkanLibrary.GetProcAddress<PFN_vkEnumeratePhysicalDevices>("vkEnumeratePhysicalDevices");
    vkGetPhysicalDeviceProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceProperties>("vkGetPhysicalDeviceProperties");
    vkGetPhysicalDeviceFeatures = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceFeatures>("vkGetPhysicalDeviceFeatures");
    vkGetPhysicalDeviceMemoryProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceMemoryProperties>("vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceQueueFamilyProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceQueueFamilyProperties>("vkGetPhysicalDeviceQueueFamilyProperties");

    vkCreateDevice = vulkanLibrary.GetProcAddress<PFN_vkCreateDevice>("vkCreateDevice");
}

}