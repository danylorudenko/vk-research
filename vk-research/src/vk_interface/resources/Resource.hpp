#pragma once

#include <vulkan\vulkan.h>

#include <limits>

#include <vk_interface\memory\Memory.hpp>

namespace VKW
{


struct BufferResource
{
    BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory);

    VkBuffer        handle_     = VK_NULL_HANDLE;
    std::uint32_t   size_       = 0;
    MemoryRegion    memory_;

    MemoryPage* GetMemoryPage() const;
};

struct SubbufferResource
{
    VkBuffer        handle_ = VK_NULL_HANDLE;
    std::uint32_t   offset_ = 0;
    std::uint32_t   size_   = 0;
};



struct ImageResource
{
    ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryRegion const& memory);

    VkImage         handle_ = VK_NULL_HANDLE;
    VkFormat        format_ = VK_FORMAT_UNDEFINED;
    std::uint32_t   width_  = 0;
    std::uint32_t   height_ = 0;
    MemoryRegion    memory_;

    MemoryPage* GetMemoryPage() const;
};

struct ImageResourceView
{
    ImageResourceView(VkImageView handle, VkImageViewType type, VkFormat format, VkComponentMapping const& componentMapping, VkImageSubresourceRange const& subresourceRange, ImageResource* parentResource);

    VkImageView             handle_         = VK_NULL_HANDLE;
    VkImageViewType         type_           = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkFormat                format_         = VK_FORMAT_UNDEFINED;
    VkComponentMapping      componentMapping_;
    VkImageSubresourceRange subresourceRange_;
    ImageResource*          parentResource_ = nullptr;

    MemoryRegion*   GetMemoryRegion() const;
    MemoryPage*     GetMemoryPage() const;
    VkImage         GetImageHandle() const;
    VkFormat        GetImageFormat() const;
    std::uint32_t   GetImageWidth() const;
    std::uint32_t   GetImageHeight() const;
};

}