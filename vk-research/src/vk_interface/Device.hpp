#pragma once

#include <vulkan/vulkan.h>

#include "..\class_features\NonCopyable.hpp"

#include "ImportTable.hpp"
#include "Instance.hpp"

namespace VKW
{

struct DeviceDesc
{
    ImportTable* table_; 
    Instance* instance_; 
    std::vector<std::string> requiredExtensions_;
};

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
    Device(DeviceDesc const& rhs);

    Device(Device&& rhs);
    Device& operator=(Device&& rhs);

    ~Device();

    VKW::Device::PhysicalDeviceProperties const& Properties() const;

    VkDevice Handle() const;
    operator bool() const;

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
    ImportTable* table_;

    VkPhysicalDevice physicalDevice_;
    VKW::Device::PhysicalDeviceProperties physicalDeviceProperties_;
};

}