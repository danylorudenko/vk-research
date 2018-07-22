#include "ImportTable.hpp"

namespace VKW
{

ImportTable::ImportTable() = default;

ImportTable::ImportTable(DynamicLibrary& vulkanLibrary)
{
    assert(vulkanLibrary);
    
    vkGetInstanceProcAddr = vulkanLibrary.GetProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vkGetDeviceProcAddr = vulkanLibrary.GetProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

    vkCreateInstance = vulkanLibrary.GetProcAddress<PFN_vkCreateInstance>("vkCreateInstance");
    vkDestroyInstance = vulkanLibrary.GetProcAddress<PFN_vkDestroyInstance>("vkDestroyInstance");

    vkEnumerateInstanceLayerProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    vkEnumerateInstanceExtensionProperties = vulkanLibrary.GetProcAddress<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
}

void ImportTable::GetInstanceProcAddresses(VkInstance instance)
{
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
    vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties"));

    vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures"));
    vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties"));
    vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));

    vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

    vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(vkGetInstanceProcAddr(instance, "vkCreateDevice"));
    vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(vkGetInstanceProcAddr(instance, "vkDeviceWaitIdle"));
    vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(vkGetInstanceProcAddr(instance, "vkDestroyDevice"));
}

void ImportTable::GetDeviceProcAddresses(VkDevice device)
{
    vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(vkGetDeviceProcAddr(device, "vkAllocateMemory"));
    vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(vkGetDeviceProcAddr(device, "vkFreeMemory"));
    vkGetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(vkGetDeviceProcAddr(device, "vkGetDeviceMemoryCommitment"));

    vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(vkGetDeviceProcAddr(device, "vkMapMemory"));
    vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(vkGetDeviceProcAddr(device, "vkUnmapMemory"));

    vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(vkGetDeviceProcAddr(device, "vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(vkGetDeviceProcAddr(device, "vkInvalidateMappedMemoryRanges"));

    vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(vkGetDeviceProcAddr(device, "vkCreateBuffer"));
    vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(vkGetDeviceProcAddr(device, "vkDestroyBuffer"));
    vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(vkGetDeviceProcAddr(device, "vkCreateImage"));
    vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(vkGetDeviceProcAddr(device, "vkDestroyImage"));

    vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceFormatProperties"));
    vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceImageFormatProperties"));

    vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements"));
    vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements"));
    vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(vkGetDeviceProcAddr(device, "vkGetImageSubresourceLayout"));

    vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(vkGetDeviceProcAddr(device, "vkBindBufferMemory"));
    vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(vkGetDeviceProcAddr(device, "vkBindImageMemory"));

    vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(vkGetDeviceProcAddr(device, "vkCreateBufferView"));
    vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(vkGetDeviceProcAddr(device, "vkDestroyBufferView"));
    vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(vkGetDeviceProcAddr(device, "vkCreateImageView"));
    vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(vkGetDeviceProcAddr(device, "vkDestroyImageView"));

    vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(vkGetDeviceProcAddr(device, "vkGetDeviceQueue"));
    
    vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(vkGetDeviceProcAddr(device, "vkCreateCommandPool"));
    vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(vkGetDeviceProcAddr(device, "vkResetCommandPool"));
    vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(vkGetDeviceProcAddr(device, "vkDestroyCommandPool"));
    
    vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers"));
    vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(vkGetDeviceProcAddr(device, "vkResetCommandBuffer"));
    vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(vkGetDeviceProcAddr(device, "vkFreeCommandBuffers"));
}

}