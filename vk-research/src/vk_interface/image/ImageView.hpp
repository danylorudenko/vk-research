#pragma once

#include <vk_interface\resources\Resource.hpp>

namespace VKW
{


struct ImageView
{
    ImageView(VkImageView view, VkFormat format, VkImageViewType type, VkImageSubresourceRange const& range, ImageResource* imageRes);

    VkImageView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkImageViewType type_;
    VkImageSubresourceRange subresourceRange_;
    ImageResource* resource_;
};

class ImageViewHandle
{
public:
    explicit ImageViewHandle();
    explicit ImageViewHandle(ImageView* view);

    ImageViewHandle(ImageViewHandle const& rhs);
    ImageViewHandle(ImageViewHandle&& rhs);

    ImageViewHandle& operator=(ImageViewHandle const& rhs);
    ImageViewHandle& operator=(ImageViewHandle&& rhs);

    ~ImageViewHandle();

    ImageView* GetView() const;

private:
    ImageView* view_;
};

}