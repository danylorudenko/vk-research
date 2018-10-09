#include "ImagesProvider.hpp"
#include "../Device.hpp"
#include "../ImportTable.hpp"
#include "../resources/ResourcesController.hpp"

#include <utility>
#include <algorithm>

namespace VKW
{

ImagesProvider::ImagesProvider()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{

}

ImagesProvider::ImagesProvider(ImagesProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ desc.resourcesController_ }
{

}

ImagesProvider::ImagesProvider(ImagesProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

ImagesProvider& ImagesProvider::operator=(ImagesProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
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
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.flags = VK_FLAGS_NONE;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 1;
    viewInfo.subresourceRange.baseMipLevel = 1;
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