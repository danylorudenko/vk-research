#include "ImageView.hpp"

namespace VKW
{

ImageView::ImageView(VkImageView view, VkFormat format, VkImageViewType type, VkImageSubresourceRange const& range, ImageResourceHandle imageRes)
    : handle_{ view }
    , format_{ format }
    , type_{ type }
    , subresourceRange_{ range }
    , resource_{ imageRes }
{

}

ImageViewHandle::ImageViewHandle()
    : view_{ nullptr }
{
}

ImageViewHandle::ImageViewHandle(ImageView* view)
    : view_{ view }
{
}

ImageViewHandle::ImageViewHandle(ImageViewHandle const& rhs) = default;

ImageViewHandle::ImageViewHandle(ImageViewHandle&& rhs) = default;

ImageViewHandle& ImageViewHandle::operator=(ImageViewHandle const& rhs) = default;

ImageViewHandle& ImageViewHandle::operator=(ImageViewHandle&& rhs) = default;

ImageViewHandle::~ImageViewHandle() = default;

ImageView* ImageViewHandle::GetView() const
{
    return view_;
}

}