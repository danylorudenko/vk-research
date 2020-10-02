#pragma once

#include <vulkan\vulkan.h>

#include <limits>

#include <vk_interface\memory\Memory.hpp>

namespace VKW
{


struct BufferResource
{
    BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory);

    VkBuffer handle_ = VK_NULL_HANDLE;
    std::uint32_t size_ = 0;
    MemoryRegion memory_;
};

struct SubbufferResource
{
    VkBuffer handle_ = VK_NULL_HANDLE;
    std::uint32_t offset_ = 0;
    std::uint32_t size_ = 0;
};

class BufferResourceHandle
{
public:
    explicit BufferResourceHandle();
    explicit BufferResourceHandle(BufferResource* resource);

    BufferResourceHandle(BufferResourceHandle const& rhs);
    BufferResourceHandle(BufferResourceHandle&& rhs);

    BufferResourceHandle& operator=(BufferResourceHandle const& rhs);
    BufferResourceHandle& operator=(BufferResourceHandle&& rhs);

    ~BufferResourceHandle();


    BufferResource* GetResource() const;
    MemoryPage*     GetMemoryPage() const;

private:
    BufferResource* resource_ = nullptr;
};



struct ImageResource
{
    ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryRegion const& memory);

    VkImage handle_ = VK_NULL_HANDLE;
    VkFormat format_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    MemoryRegion memory_;
};

struct ImageResourceHandle
{
public:
    explicit ImageResourceHandle();
    explicit ImageResourceHandle(ImageResource* resource);

    ImageResourceHandle(ImageResourceHandle const& rhs);
    ImageResourceHandle(ImageResourceHandle&& rhs);

    ImageResourceHandle& operator=(ImageResourceHandle const& rhs);
    ImageResourceHandle& operator=(ImageResourceHandle&& rhs);

    ~ImageResourceHandle();

    ImageResource*  GetResource() const;
    MemoryPage*     GetMemoryPage() const;

private:
    ImageResource* resource_ = nullptr;
};

}