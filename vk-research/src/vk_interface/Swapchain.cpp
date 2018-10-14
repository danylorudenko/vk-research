#include "Swapchain.hpp"

#include <utility>

namespace VKW
{

Swapchain::Swapchain()
    : table_{ nullptr }
    , instance_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ VK_NULL_HANDLE }
{

}

Swapchain::Swapchain(SwapchainDesc const& desc)
    : table_{ desc.table_ }
    , instance_{ desc.instance_ }
    , device_{ desc.device_ }
    , swapchain_{ VK_NULL_HANDLE }
{
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    swapchain_ = swapChain;

}

Swapchain::Swapchain(Swapchain&& rhs)
    : table_{ nullptr }
    , instance_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

Swapchain& Swapchain::operator=(Swapchain&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(instance_, rhs.instance_);
    std::swap(device_, rhs.device_);

    std::swap(swapchain_, rhs.swapchain_);
    std::swap(swapChainImages_, rhs.swapChainImages_);

    return *this;
}

Swapchain::~Swapchain()
{
    if (swapchain_) {

    }
}

}