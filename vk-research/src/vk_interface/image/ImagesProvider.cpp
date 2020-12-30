#include "ImagesProvider.hpp"

#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Swapchain.hpp>
#include <vk_interface\resources\ResourcesController.hpp>
#include <vk_interface\VkInterfaceConstants.hpp>

#include <utility>
#include <algorithm>

namespace VKW
{

ImagesProvider::ImagesProvider()
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , defaultSampler_{ VK_NULL_HANDLE }
    , resourcesController_{ nullptr }
{

}

ImagesProvider::ImagesProvider(ImagesProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , swapchain_{ desc.swapchain_ }
    , defaultSampler_{ VK_NULL_HANDLE }
    , resourcesController_{ desc.resourcesController_ }
{
    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = VK_FLAGS_NONE;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0.0f;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f; // TODO: ???????????????
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VK_ASSERT(table_->vkCreateSampler(device_->Handle(), &samplerInfo, nullptr, &defaultSampler_));
 
}

ImagesProvider::ImagesProvider(ImagesProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , defaultSampler_{ VK_NULL_HANDLE }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

ImagesProvider& ImagesProvider::operator=(ImagesProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(swapchain_, rhs.swapchain_);
    std::swap(defaultSampler_, rhs.defaultSampler_);
    std::swap(resourcesController_, rhs.resourcesController_);

    std::swap(imageViewContainers_, rhs.imageViewContainers_);

    return *this;
}

ImagesProvider::~ImagesProvider()
{
    VkDevice const device = device_->Handle();
    
    for (auto const& imageViewCont : imageViewContainers_) {
        ImageView* imageView = imageViewCont.view_;
        ImageResource* imageResource = imageView->resource_;
        table_->vkDestroyImageView(device, imageView->handle_, nullptr);
        delete imageView;

        if (!imageViewCont.isSwapchain_)
            resourcesController_->FreeImage(imageResource);
    }

    if (defaultSampler_ != VK_NULL_HANDLE) {
        table_->vkDestroySampler(device, defaultSampler_, nullptr);
        defaultSampler_ = VK_NULL_HANDLE;
    }
}

VkSampler ImagesProvider::DefaultSamplerHandle() const
{
    return defaultSampler_;
}

ImageView* ImagesProvider::RegisterSwapchainImageView(std::uint32_t index)
{
    VkImageView vkImageView = VK_NULL_HANDLE;

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = swapchain_->Image(index).image_;
    viewInfo.format = swapchain_->Format();
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.flags = VK_FLAGS_NONE;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    VK_ASSERT(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &vkImageView));

    ImageView* imageView = new ImageView{ vkImageView, swapchain_->Format(), VK_IMAGE_VIEW_TYPE_2D, viewInfo.subresourceRange, nullptr };
    
    imageViewContainers_.emplace_back(ImageViewContainer{ imageView, true });
    
    return imageView;
}

std::vector<ImageView*> ImagesProvider::AllocateImagesAndViews(std::uint32_t count, VkFormat format, std::uint32_t width, std::uint32_t height, ImageUsage usage)
{
    std::vector<ImageView*> results;
    results.reserve(count);

    ImageDesc imageDesc;
    imageDesc.format_ = format;
    imageDesc.width_ = width;
    imageDesc.height_ = height;
    imageDesc.usage_ = usage;

    VkImageAspectFlags aspectFlags;
    switch (usage) {
    case ImageUsage::RENDER_TARGET:
    case ImageUsage::TEXTURE:
    case ImageUsage::STORAGE_IMAGE:
    case ImageUsage::STORAGE_IMAGE_READONLY:
    case ImageUsage::UPLOAD_IMAGE:
        aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        break;
    case ImageUsage::DEPTH:
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case ImageUsage::STENCIL:
        aspectFlags = VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    case ImageUsage::DEPTH_STENCIL:
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        assert(false && "Image usage not supported");
    }

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.image = VK_NULL_HANDLE;
    viewInfo.format = format;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.flags = VK_FLAGS_NONE;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    for (std::uint32_t i = 0; i < count; ++i) {
        ImageResource* imageResource = resourcesController_->CreateImage(imageDesc);
        viewInfo.image = imageResource->handle_;
        VkImageView vkView = VK_NULL_HANDLE;

        if (usage != ImageUsage::UPLOAD_IMAGE)
            VK_ASSERT(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &vkView));

        ImageView* imageView = new ImageView{ vkView, viewInfo.format, viewInfo.viewType, viewInfo.subresourceRange, imageResource };

        imageViewContainers_.emplace_back(ImageViewContainer{ imageView, false });

        results.emplace_back(imageView);
    }

    return results;
}

void ImagesProvider::ReleaseImagesAndViews(std::uint32_t count, ImageView** handles)
{
    for (std::uint32_t i = 0; i < count; ++i) {
        auto imageViewContIt = std::find_if(
            imageViewContainers_.begin(),
            imageViewContainers_.end(),
            [handles, i](ImageViewContainer const& container) {
            return container.view_ == handles[i];
        });

        assert(imageViewContIt != imageViewContainers_.end() && "Can't release ImageView.");

        ImageView* imageView = imageViewContIt->view_;
        ImageResource* imageResource = imageView->resource_;
        table_->vkDestroyImageView(device_->Handle(), imageView->handle_, nullptr);
        delete imageView;

        resourcesController_->FreeImage(imageResource);

        imageViewContainers_.erase(imageViewContIt);
    }
}

void ImagesProvider::ReleaseImagesAndViews(std::vector<ImageView*> handles)
{
    ReleaseImagesAndViews((std::uint32_t)handles.size(), handles.data());
}

}