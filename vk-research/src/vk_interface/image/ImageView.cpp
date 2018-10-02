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

}