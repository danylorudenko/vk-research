#include "Swapchain.hpp"

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Surface.hpp>
#include <vk_interface\Tools.hpp>

#include <algorithm>

namespace VKW
{

Swapchain::Swapchain()
    : table_{ nullptr }
    , device_{ nullptr }
    , surface_{ nullptr }
    , swapchain_{ VK_NULL_HANDLE }
    , swapchainFormat_{ VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR }
    , width_{ 0 }
    , height_{ 0 }
    , swapchainImageCount_{ 0 }
{
}

Swapchain::Swapchain(SwapchainDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , surface_{ desc.surface_ }
    , swapchain_{ VK_NULL_HANDLE }
    , swapchainFormat_{ VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR }
    , width_{ 0 }
    , height_{ 0 }
    , swapchainImageCount_{ 0 }
{
    VkDevice const device = device_->Handle();

    auto const& surfaceFormats = surface_->SurfaceFormats();
    assert(surfaceFormats.size() > 0 && "Surface supportes no formats!");
    assert(surface_->SupportedQueueFamilies().size() > 0 && "Surface doesn't support any queue families on the initialized device.");

    auto const& validSurfaceFormat = surfaceFormats[0];

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.surface = surface_->Handle();
    swapchainInfo.minImageCount = desc.imagesCount_;
    swapchainInfo.imageFormat = validSurfaceFormat.format;
    swapchainInfo.imageColorSpace = validSurfaceFormat.colorSpace;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    // transfer usage to enable vkCmdClearImage out of VkRenderPass scope
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; 
    swapchainInfo.imageExtent = surface_->SurfaceCapabilities().currentExtent;
    swapchainInfo.clipped = VK_FALSE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.flags = VK_FLAGS_NONE;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.preTransform = surface_->SurfaceCapabilities().currentTransform;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    
    VK_ASSERT(table_->vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain_));

    swapchainFormat_ = validSurfaceFormat;
    width_ = surface_->SurfaceCapabilities().currentExtent.width;
    height_ = surface_->SurfaceCapabilities().currentExtent.height;


    std::vector<VkImage> swapchainImages;
    std::uint32_t swapchainImagesCount = 0;

    VK_ASSERT(table_->vkGetSwapchainImagesKHR(device, swapchain_, &swapchainImagesCount, nullptr));
    swapchainImages.resize(swapchainImagesCount);
    swapchainImageCount_ = swapchainImagesCount;
    VK_ASSERT(table_->vkGetSwapchainImagesKHR(device, swapchain_, &swapchainImagesCount, swapchainImages.data()));

    for (auto i = 0u; i < swapchainImagesCount; ++i) {
        
        SwapchainImage swapchainImageWrapper;
        swapchainImageWrapper.image_ = swapchainImages[i];
        
        swapchainImages_.push_back(swapchainImageWrapper);
    }
}

Swapchain::Swapchain(Swapchain&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , surface_{ nullptr }
    , swapchain_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

Swapchain& Swapchain::operator=(Swapchain&& rhs)
{
/*
    ImportTable* table_;
    Device* device_;
    Surface* surface_;

    VkSwapchainKHR swapchain_;
    VkSurfaceFormatKHR swapchainFormat_;
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t swapchainImageCount_;
    std::vector<SwapchainImage> swapchainImages_;*/
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(surface_, rhs.surface_);

    std::swap(swapchain_, rhs.swapchain_);
    std::swap(swapchainFormat_, rhs.swapchainFormat_);
    std::swap(width_, rhs.width_);
    std::swap(height_, rhs.height_);
    std::swap(swapchainImageCount_, rhs.swapchainImageCount_);
    std::swap(swapchainImages_, rhs.swapchainImages_);

    return *this;
}

Swapchain::~Swapchain()
{
    if (swapchain_) {
        table_->vkDestroySwapchainKHR(device_->Handle(), swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

VkSwapchainKHR Swapchain::Handle() const
{
    return swapchain_;
}

VkFormat Swapchain::Format() const
{
    return swapchainFormat_.format;
}

std::uint32_t Swapchain::Width() const
{
    return width_;
}

std::uint32_t Swapchain::Height() const
{
    return height_;
}

std::uint32_t Swapchain::ImageCount() const
{
    return swapchainImageCount_;
}

Swapchain::SwapchainImage& Swapchain::Image(std::uint32_t index)
{
    return swapchainImages_[index];
}


}