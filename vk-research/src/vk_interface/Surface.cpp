#include "Surface.hpp"
#include "Device.hpp"
#include "Tools.hpp"
#include "Instance.hpp"

#include <utility>

namespace VKW
{

Surface::Surface()
    : table_{ nullptr }
    , instance_{ nullptr }
    , device_{ nullptr }
    , surface_{ VK_NULL_HANDLE }
{

}

Surface::Surface(SurfaceDesc const& desc)
    : table_{ desc.table_ }
    , instance_{ desc.instance_ }
    , device_{ desc.device_ }
    , surface_{ VK_NULL_HANDLE }
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR sInfo;
    sInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sInfo.pNext = nullptr;
    sInfo.flags = VK_FLAGS_NONE;
    sInfo.hinstance = desc.hInstance_;
    sInfo.hwnd = desc.hwnd_;
    
    VK_ASSERT(table_->vkCreateWin32SurfaceKHR(instance_->Handle(), &sInfo, nullptr, &surface));
    surface_ = surface;
#endif

    if (surface) {
        VkSurfaceCapabilitiesKHR surfaceCaps;
        table_->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_->PhysicalDeviceHandle(), surface_, &surfaceCaps);

        surfaceCapabilities_ = surfaceCaps;
    }
}

Surface::Surface(Surface&& rhs)
    : table_{ nullptr }
    , instance_{ nullptr }
    , device_{ nullptr }
    , surface_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

Surface& Surface::operator=(Surface&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(instance_, rhs.instance_);
    std::swap(device_, rhs.device_);

    std::swap(surface_, rhs.surface_);
    std::swap(surfaceCapabilities_, rhs.surfaceCapabilities_);
    std::swap(presentMode_, rhs.presentMode_);
    std::swap(surfaceFormat_, rhs.surfaceFormat_);

    return *this;
}

Surface::operator bool() const
{
    return surface_ != VK_NULL_HANDLE;
}

VkSurfaceKHR Surface::Handle() const
{
    return surface_;
}

VkSurfaceCapabilitiesKHR const& Surface::SurfaceCapabilities() const
{
    return surfaceCapabilities_;
}

VkPresentModeKHR Surface::PresentMode() const
{
    return presentMode_;
}

VkSurfaceFormatKHR const& Surface::SurfaceFormat() const
{
    return surfaceFormat_;
}

Surface::~Surface()
{
    if (surface_ != VK_NULL_HANDLE) {
        table_->vkDestroySurfaceKHR(instance_->Handle(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

}