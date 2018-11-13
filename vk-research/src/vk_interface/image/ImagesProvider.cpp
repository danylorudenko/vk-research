#include "ImagesProvider.hpp"
#include "..\Device.hpp"
#include "..\ImportTable.hpp"
#include "..\resources\ResourcesController.hpp"

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

    VK_ASSERT(table_->vkCreateSampler(device_->Handle(), &samplerInfo, nullptr, &defaultSampler_));

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

    std::swap(imageViewContainers_, rhs.imageViewContainers_);

    return *this;
}

ImagesProvider::~ImagesProvider()
{
    VkDevice const device = device_->Handle();
    
    for (auto const& imageViewCont : imageViewContainers_) {
        ImageView* imageView = imageViewCont.view_;
        table_->vkDestroyImageView(device, imageView->handle_, nullptr);
        delete imageView;

        resourcesController_->FreeImage(imageViewCont.resource_);
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

ImageView* ImagesProvider::GetImageView(ImageViewHandle handle)
{
    return handle.view_;
}

ImageViewHandle ImagesProvider::AcquireImage(ImageViewDesc const& desc)
{
    ImageDesc imageDesc;
    imageDesc.format_ = desc.format_;
    imageDesc.width_ = desc.width_;
    imageDesc.height_ = desc.height_;
    imageDesc.usage_ = desc.usage_;

    ImageResourceHandle imageResourceHandle = resourcesController_->CreateImage(imageDesc);
    ImageResource* imageResource = resourcesController_->GetImage(imageResourceHandle);

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.image = imageResource->handle_;
    viewInfo.format = desc.format_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.flags = VK_FLAGS_NONE;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    VkImageView vkView = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateImageView(device_->Handle(), &viewInfo, nullptr, &vkView));

    ImageView* imageView = new ImageView{ vkView, viewInfo.format, viewInfo.viewType, viewInfo.subresourceRange, imageResourceHandle };
    
    ImageViewContainer container;
    container.view_ = imageView;
    container.resource_ = imageResourceHandle;

    imageViewContainers_.emplace_back(container);

    return ImageViewHandle{ imageView };
}

void ImagesProvider::ReleaseImage(ImageViewHandle handle)
{
    auto imageViewContIt = std::find_if(
        imageViewContainers_.begin(), 
        imageViewContainers_.end(),
        [handle](ImageViewContainer const& container) {
            return container.view_ == handle.view_;
        });

    assert(imageViewContIt != imageViewContainers_.end() && "Can't release ImageView.");

    ImageView* imageView = imageViewContIt->view_;
    table_->vkDestroyImageView(device_->Handle(), imageView->handle_, nullptr);
    delete imageView;

    resourcesController_->FreeImage(imageViewContIt->resource_);

    imageViewContainers_.erase(imageViewContIt);
}

}