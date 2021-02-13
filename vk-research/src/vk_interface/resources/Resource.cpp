#include "Resource.hpp"

namespace VKW
{

BufferResource::BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory)
    : handle_{ handle }, size_{ size }, memory_{ memory }
{

}

MemoryPage* BufferResource::GetMemoryPage() const
{
    return memory_.page_;
}

ImageResource::ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryRegion const& memory)
    : handle_{ handle }
    , format_{ format }
    , width_{ width }
    , height_{ height }
    , memory_{ memory }
{

}

MemoryPage* ImageResource::GetMemoryPage() const
{
    return memory_.page_;
}

ImageResourceView::ImageResourceView(VkImageView handle, VkImageViewType type, VkFormat format, VkComponentMapping const& componentMapping, VkImageSubresourceRange const& subresourceRange, ImageResource* parentResource)
    : handle_{ handle }
    , type_{ type }
    , format_{ format }
    , componentMapping_{ componentMapping }
    , subresourceRange_{ subresourceRange }
    , parentResource_{ parentResource }
{
}

MemoryRegion* ImageResourceView::GetMemoryRegion() const
{
    return &parentResource_->memory_;
}

MemoryPage* ImageResourceView::GetMemoryPage() const
{
    return parentResource_->GetMemoryPage();
}

VkImage ImageResourceView::GetImageHandle() const
{
    return parentResource_->handle_;
}

VkFormat ImageResourceView::GetImageFormat() const
{
    return parentResource_->format_;
}

std::uint32_t ImageResourceView::GetImageWidth() const
{
    return parentResource_->width_;
}

std::uint32_t ImageResourceView::GetImageHeight() const
{
    return parentResource_->height_;
}

}