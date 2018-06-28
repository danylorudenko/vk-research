#include "Device.hpp"
#include "Tools.hpp"

namespace VKW
{

Device::Device()
    : device_{ VK_NULL_HANDLE }
    , table_{ nullptr }
{
}

Device::Device(VulkanImportTable* table, Instance& instance, std::vector<std::string> const& requiredExtensions)
    : device_{ VK_NULL_HANDLE }
    , table_{ table }
{
    std::uint32_t physicalDeviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;
    {
        VK_ASSERT(table_->vkEnumeratePhysicalDevices(instance.Handle(), &physicalDeviceCount, nullptr));
        physicalDevices.resize(physicalDeviceCount);
        VK_ASSERT(table_->vkEnumeratePhysicalDevices(instance.Handle(), &physicalDeviceCount, physicalDevices.data()));
    }
    
    {
        VkPhysicalDevice primaryDevice = VK_NULL_HANDLE;

        auto* properties = new VkPhysicalDeviceProperties{ 0 };
        auto* memoryProperties = new VkPhysicalDeviceMemoryProperties{ 0 };

        for (auto i = 0; i < physicalDeviceCount; ++i) {
            
            table_->vkGetPhysicalDeviceProperties(physicalDevices[i], properties);
            std::cout 
                << "PHYSICAL DEVICE: " << i << std::endl 
                << "Name: " << properties->deviceName << std::endl;

            // priority to Nvidia devices.
            if (properties->vendorID == VENDOR_ID_NVIDIA) {
                primaryDevice = physicalDevices[i];
            }

            table_->vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], memoryProperties);

            *properties = VkPhysicalDeviceProperties{ 0 };
            *memoryProperties = VkPhysicalDeviceMemoryProperties{ 0 };
        }
        delete properties;
        delete memoryProperties;
    }

}

Device::Device(Device&& rhs)
    : device_{ VK_NULL_HANDLE }
    , table_{ nullptr }
{
    operator=(std::move(rhs));
}

Device& Device::operator=(Device&& rhs)
{
    std::swap(device_, rhs.device_);
    std::swap(table_, rhs.table_);

    return *this;
}

VkDevice Device::Handle() const
{
    return device_;
}

Device::operator bool() const
{
    return device_ != VK_NULL_HANDLE;
}

}