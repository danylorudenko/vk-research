#include "Resource.hpp"

namespace VKW
{

BufferResource::BufferResource(VkBuffer handle, std::uint32_t size, MemoryRegion const& memory)
    : handle_{ handle }, size_{ size }, memory_{ memory }
{

}

}