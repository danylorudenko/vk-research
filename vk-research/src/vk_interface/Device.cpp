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
        auto* properties = new VkPhysicalDeviceProperties{ 0 };
        for (auto i = 0; i < physicalDeviceCount; ++i) {
            
            table_->vkGetPhysicalDeviceProperties(physicalDevices[i], properties);
            std::cout 
                << "PHYSICAL DEVICE: " << i << std::endl 
                << "Name: " << properties->deviceName << std::endl;
            
            *properties = VkPhysicalDeviceProperties{ 0 };
        }
        delete properties;
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

}