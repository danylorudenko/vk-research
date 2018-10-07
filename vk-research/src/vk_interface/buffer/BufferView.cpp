#include "BufferView.hpp"

namespace VKW
{

BufferView::BufferView(VkBufferView view, VkFormat format, std::uint64_t offset, std::uint64_t size, ProvidedBuffer* providedBuffer)
    : handle_{ view }
    , format_{ format }
    , offset_{ offset }
    , size_{ size_ }
    , providedBuffer_{ providedBuffer }
{

}

}