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
    , defaultSampler_{ VK_NULL_HANDLE }
    , resourcesController_{ nullptr }
{

}

ImagesProvider::ImagesProvider(ImagesProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
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
    samplerInfo.maxLod = 1.0f; // ????????
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    ERR_GUARD_VK(table_->vkCreateSampler(device_->Handle(), &samplerInfo, nullptr, &defaultSampler_));
 
}

ImagesProvider::ImagesProvider(ImagesProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , defaultSampler_{ VK_NULL_HANDLE }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

ImagesProvider& ImagesProvider::operator=(ImagesProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(defaultSampler_, rhs.defaultSampler_);
    std::swap(resourcesController_, rhs.resourcesController_);

    std::swap(imageViews_, rhs.imageViews_);

    return *this;
}

ImagesProvider::~ImagesProvider()
{
    VkDevice const device = device_->Handle();
    
    for (ImageView* imageView : imageViews_) {
        ImageResource* imageResource = imageView->resource_;
        table_->vkDestroyImageView(device, imageView->handle_, nullptr);
        delete imageView;

        // HACK: case for swapchain view
        if(imageResource != nullptr)
        {
            resourcesController_->FreeImage(imageResource);
        }
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

ImageView* ImagesProvider::RegisterSwapchainImageView(SwapchainImageViewDesc const& desc, Swapchain* swapchain)
{
    VkImageView vkImageView = VK_NULL_HANDLE;

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = swapchain->Image(desc.index_).image_;
    viewInfo.format = swapchain->Format();
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.flags = VK_FLAGS_NONE;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    ERR_GUARD_VK(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &vkImageView));

    ImageView* imageView = new ImageView{ vkImageView, swapchain->Format(), VK_IMAGE_VIEW_TYPE_2D, viewInfo.subresourceRange, nullptr };
    
    imageViews_.emplace(imageView);
    
    return imageView;
}

void ImagesProvider::CreateViewsAndCreateImages(std::uint32_t count, ImageViewDesc const* descs, ImageView** results)
{
    for (std::uint32_t i = 0; i < count; ++i) {
        ImageDesc imageDesc;
        imageDesc.format_ = descs[i].format_;
        imageDesc.width_ = descs[i].width_;
        imageDesc.height_ = descs[i].height_;
        imageDesc.usage_ = descs[i].usage_;
        ImageResource* imageResource = resourcesController_->CreateImage(imageDesc);

        VkImageAspectFlags aspectFlags;
        switch (descs[i].usage_) {
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
        viewInfo.image = imageResource->handle_;
        viewInfo.format = descs[i].format_;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.levelCount = 1;

        VkImageView vkView = VK_NULL_HANDLE;

        if (descs[i].usage_ != ImageUsage::UPLOAD_IMAGE)
            ERR_GUARD_VK(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &vkView));

        ImageView* imageView = new ImageView{ vkView, viewInfo.format, viewInfo.viewType, viewInfo.subresourceRange, imageResource };

        imageViews_.emplace(imageView);

        results[i] = imageView;
    }
}

void ImagesProvider::ReleaseViewsAndImages(std::uint32_t count, ImageView** views)
{
    for (std::uint32_t i = 0; i < count; ++i) {
        ImageView* imageView = views[i];

        auto result = imageViews_.erase(imageView);
        assert(result > 0 && "Failed to release ImageView.");

        table_->vkDestroyImageView(device_->Handle(), imageView->handle_, nullptr);

        resourcesController_->FreeImage(imageView->resource_);
        delete imageView;
        views[i] = nullptr;
    }
}

}