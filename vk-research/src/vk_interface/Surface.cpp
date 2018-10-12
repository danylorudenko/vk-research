#include "Surface.hpp"
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
#endif
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

    return *this;
}

Surface::~Surface()
{
    if (surface_ != VK_NULL_HANDLE) {
        table_->vkDestroySurfaceKHR(instance_->Handle(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

}