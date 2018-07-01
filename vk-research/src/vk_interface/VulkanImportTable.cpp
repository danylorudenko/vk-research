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
    vkDestroyInstance = vulkanLibrary.GetProcAddress<PFN_vkDestroyInstance>("vkDestroyInstance");

    vkEnumerateInstanceLayerProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    vkEnumerateInstanceExtensionProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
    
    vkGetPhysicalDeviceProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceProperties>("vkGetPhysicalDeviceProperties");
    vkGetPhysicalDeviceFeatures = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceFeatures>("vkGetPhysicalDeviceFeatures");
    vkGetPhysicalDeviceMemoryProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceMemoryProperties>("vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceQueueFamilyProperties = vulkanLibrary.GetProcAddress<PFN_vkGetPhysicalDeviceQueueFamilyProperties>("vkGetPhysicalDeviceQueueFamilyProperties");
    
    vkEnumerateDeviceExtensionProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateDeviceExtensionProperties>("vkEnumerateDeviceExtensionProperties");
    vkCreateDevice = vulkanLibrary.GetProcAddress<PFN_vkCreateDevice>("vkCreateDevice");
}

void VulkanImportTable::GetInstanceProcAddresses(VkInstance instance)
{
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
}

void VulkanImportTable::GetDeviceProcAddresses(VkDevice device)
{
    //////////
}

}