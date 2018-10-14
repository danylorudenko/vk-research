#include "Swapchain.hpp"
#include "ImportTable.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Tools.hpp"

#include <utility>
#include <algorithm>

namespace VKW
{

Swapchain::Swapchain()
    : table_{ nullptr }
    , device_{ nullptr }
    , surface_{ nullptr }
    , swapchain_{ VK_NULL_HANDLE }
{

}

Swapchain::Swapchain(SwapchainDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , surface_{ desc.surface_ }
    , swapchain_{ VK_NULL_HANDLE }
{
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
    swapchainInfo.clipped = true;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.flags = VK_FLAGS_NONE;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.preTransform = surface_->SurfaceCapabilities().currentTransform;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    
    VK_ASSERT(table_->vkCreateSwapchainKHR(device_->Handle(), &swapchainInfo, nullptr, &swapchain_));



    std::vector<VkImage> swapchainImages;
    std::uint32_t swapchainImagesCount = 0;

    VK_ASSERT(table_->vkGetSwapchainImagesKHR(device_->Handle(), swapchain_, &swapchainImagesCount, nullptr));
    swapchainImages.resize(swapchainImagesCount);
    VK_ASSERT(table_->vkGetSwapchainImagesKHR(device_->Handle(), swapchain_, &swapchainImagesCount, swapchainImages.data()));

    for (auto i = 0u; i < swapchainImagesCount; ++i) {
        VkImageView imageView = VK_NULL_HANDLE;
        
        VkImageViewCreateInfo viewInfo;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = nullptr;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.image = swapchainImages[i];
        viewInfo.format = validSurfaceFormat.format;
        viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.levelCount = 1;

        VK_ASSERT(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &imageView));

        SwapchainImage swapchainImageWrapper;
        swapchainImageWrapper.image_ = swapchainImages[i];
        swapchainImageWrapper.view_ = imageView;

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
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(surface_, rhs.surface_);

    std::swap(swapchain_, rhs.swapchain_);
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

}