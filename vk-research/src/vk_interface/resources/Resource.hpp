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
    ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryRegion const& memory, VkImageCreateInfo const& createInfo);

    VkImage             handle_ = VK_NULL_HANDLE;
    VkFormat            format_ = VK_FORMAT_UNDEFINED;
    std::uint32_t       width_  = 0;
    std::uint32_t       height_ = 0;
    MemoryRegion        memory_;
    VkImageCreateInfo   createInfo_;

    MemoryPage* GetMemoryPage() const;
};

struct ImageResourceView
{
    ImageResourceView(VkImageView handle, VkImageViewCreateInfo const& createInfo, ImageResource* parentResource);

    VkImageView             handle_         = VK_NULL_HANDLE;
    VkImageViewCreateInfo   createInfo_;
    ImageResource*          parentResource_ = nullptr;

    inline VkFormat        GetFormat() const        { return createInfo_.format; }
    inline VkImageViewType GetType() const          { return createInfo_.viewType; };
    inline std::uint32_t   GetMipCount() const      { return createInfo_.subresourceRange.levelCount; }
    inline std::uint32_t   GetLayerCount() const    { return createInfo_.subresourceRange.layerCount; }
    
    inline MemoryRegion*   GetMemoryRegion() const  { return &parentResource_->memory_; }
    inline MemoryPage*     GetMemoryPage() const    { return parentResource_->GetMemoryPage(); }
    inline VkImage         GetImageHandle() const   { return parentResource_->handle_; }
    inline VkFormat        GetImageFormat() const   { return parentResource_->format_; }
    inline std::uint32_t   GetImageWidth() const    { return parentResource_->width_; }
    inline std::uint32_t   GetImageHeight() const   { return parentResource_->height_; }
};

}