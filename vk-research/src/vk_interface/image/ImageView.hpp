#pragma once

#include "../resources/Resource.hpp"

namespace VKW
{

struct ImageView
{
    ImageView(VkImageView view, VkFormat format, VkImageViewType type, VkImageSubresourceRange const& range, ImageResourceHandle imageRes);

    VkImageView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkImageViewType type_;
    VkImageSubresourceRange subresourceRange_;
    ImageResourceHandle resource_;
};

struct ImageViewHandle
{
    ImageView* view_;
};

}