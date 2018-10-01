#pragma once

#include "../resources/Resource.hpp"

namespace VKW
{

struct ImageViewHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct ImageView
{
    VkImageView handle_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkImageViewType type_;
    VkImageSubresourceRange subresourceRange_;
    ImageResourceHandle resource_;
};

}