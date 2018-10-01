#include "BufferView.hpp"

namespace VKW
{

BufferView::BufferView(VkBufferView view, VkFormat format, BufferResourceHandle buffer, std::uint64_t offset, std::uint64_t size)
    : handle_{ view }
    , format_{ format }
    , resource_{ buffer }
    , offset_{ offset }
    , size_{ size_ }
{

}

}