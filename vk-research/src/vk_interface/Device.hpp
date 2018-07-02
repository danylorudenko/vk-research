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
    class PhysicalDeviceProperties
    {
    public:
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        std::vector<VkExtensionProperties> extensionProperties;
        VkPhysicalDeviceFeatures features;
    };

public:
    Device();
    Device(VulkanImportTable* table, Instance& instance, std::vector<std::string> const& requiredExtensions);

    Device(Device&& rhs);
    Device& operator=(Device&& rhs);

    ~Device();

    operator bool() const;

    VkDevice Handle() const;

private:
    static void PrintPhysicalDeviceData(
        VKW::Device::PhysicalDeviceProperties const& deviceProperties);

    static bool IsPhysicalDeviceValid(
        VKW::Device::PhysicalDeviceProperties const& deviceProperties,
        std::vector<std::string> const& requiredExtensions
    );

    void RequestDeviceProperties(
        VkPhysicalDevice targetDevice,
        VKW::Device::PhysicalDeviceProperties& deviceProperties
    );

private:
    VkDevice device_;
    VulkanImportTable* table_;

    VkPhysicalDevice physicalDevice_;
    VKW::Device::PhysicalDeviceProperties physicalDeviceProperties_;

};

}