#include "Resource.hpp"

namespace VKW
{

BufferResource::BufferResource(VkBuffer handle, std::uint32_t size, MemoryPageRegion const& memory)
    : handle_{ handle }, size_{ size }, memoryRegion_{ memory }
{

}

BufferResourceHandle::BufferResourceHandle()
    : resource_{ nullptr }
{
}

BufferResourceHandle::BufferResourceHandle(BufferResource* resource)
    : resource_{ resource }
{
}

BufferResourceHandle::BufferResourceHandle(BufferResourceHandle const& rhs) = default;

BufferResourceHandle::BufferResourceHandle(BufferResourceHandle&& rhs) = default;

BufferResourceHandle& BufferResourceHandle::operator=(BufferResourceHandle const& rhs) = default;

BufferResourceHandle& BufferResourceHandle::operator=(BufferResourceHandle&& rhs) = default;

BufferResourceHandle::~BufferResourceHandle() = default;

BufferResource* BufferResourceHandle::GetResource() const
{
    return resource_;
}

MemoryPage* BufferResourceHandle::GetMemoryPage() const
{
    return resource_->memoryRegion_.pageHandle_.GetPage();
}

MemoryPageRegion const& BufferResourceHandle::GetMemoryPageRegion() const
{
    return resource_->memoryRegion_;
}


ImageResource::ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryPageRegion const& memory)
    : handle_{ handle }
    , format_{ format }
    , width_{ width }
    , height_{ height }
    , memoryRegion_{ memory }
{

}

ImageResourceHandle::ImageResourceHandle()
    : resource_{ nullptr }
{
}

ImageResourceHandle::ImageResourceHandle(ImageResource* resource)
    : resource_{ resource }
{
}

ImageResourceHandle::ImageResourceHandle(ImageResourceHandle const& rhs) = default;

ImageResourceHandle::ImageResourceHandle(ImageResourceHandle&& rhs) = default;

ImageResourceHandle& ImageResourceHandle::operator=(ImageResourceHandle const& rhs) = default;

ImageResourceHandle& ImageResourceHandle::operator=(ImageResourceHandle&& rhs) = default;

ImageResourceHandle::~ImageResourceHandle() = default;

ImageResource* ImageResourceHandle::GetResource() const
{
    return resource_;
}

MemoryPage* ImageResourceHandle::GetMemoryPage() const
{
    return resource_->memoryRegion_.pageHandle_.GetPage();
}

MemoryPageRegion const& ImageResourceHandle::GetMemoryPageRegion() const
{
    return resource_->memoryRegion_;
}

}