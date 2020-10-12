#include "Resource.hpp"

namespace VKW
{

BufferResource::BufferResource(VkBuffer handle, std::uint32_t size, MemoryPageRegion const& memory)
    : handle_{ handle }, size_{ size }, memoryRegion_{ memory }
{

}

ImageResource::ImageResource(VkImage handle, VkFormat format, std::uint32_t width, std::uint32_t height, MemoryPageRegion const& memory)
    : handle_{ handle }
    , format_{ format }
    , width_{ width }
    , height_{ height }
    , memoryRegion_{ memory }
{

}

}