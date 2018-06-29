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
    static std::uint32_t constexpr VENDOR_ID_NVIDIA = 0x10DE;
    static std::uint32_t constexpr VENDOR_ID_AMD = 0x1002;
    static std::uint32_t constexpr VENDOR_ID_INTEL = 0x8086;
    static std::uint32_t constexpr VENDOR_ID_ARM = 0x13B5;


public:
    Device();
    Device(VulkanImportTable* table, Instance& instance, std::vector<std::string> const& requiredExtensions);

    Device(Device&& rhs);
    Device& operator=(Device&& rhs);

    operator bool() const;

    VkDevice Handle() const;

private:
    static void PrintPhysicalDeviceData(
        VkPhysicalDeviceProperties const& properties, 
        VkPhysicalDeviceMemoryProperties const& memoryProperties,
        std::uint32_t queuePropertiesCount,
        VkQueueFamilyProperties* queueFamilyProperties,
        VkPhysicalDeviceFeatures const& features);

private:
    VkDevice device_;
    VulkanImportTable* table_;

    VkPhysicalDevice physicalDevice_;
    VkPhysicalDeviceProperties physicalDeviceProperties_;

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties_;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties_;

    VkPhysicalDeviceFeatures physicalDeviceFeatures_;

};

}