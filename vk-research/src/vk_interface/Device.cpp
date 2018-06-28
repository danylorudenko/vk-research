#include "Device.hpp"

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