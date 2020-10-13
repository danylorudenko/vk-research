#include "BufferView.hpp"

namespace VKW
{

BufferView::BufferView(VkBufferView view, VkFormat format, std::uint64_t offset, std::uint64_t size, BufferResource* bufferResource, std::uint32_t* counter)
    : handle_{ view }
    , format_{ format }
    , offset_{ offset }
    , size_{ size }
    , bufferResource_{ bufferResource }
    , bufferResourceReferenceCount_{ counter }
{

}

}