#include "ByteBuffer.hpp"

#include <stdlib.h>
#include <cstring>
#include <utility>

ByteBuffer::ByteBuffer()
    : buffer_{ nullptr }
    , size_{ 0 }
{}

ByteBuffer::ByteBuffer(std::uint64_t size)
    : buffer_{ nullptr }
    , size_{ 0 }
{
    Resize(size);
}

ByteBuffer::ByteBuffer(ByteBuffer const& rhs)
    : buffer_{ nullptr }
    , size_{ 0 }
{
    operator=(rhs);
}

ByteBuffer::ByteBuffer(ByteBuffer&& rhs)
    : buffer_{ nullptr }
    , size_{ 0 }
{
    operator=(std::move(rhs));
}

ByteBuffer& ByteBuffer::operator=(ByteBuffer const& rhs)
{
    auto size = rhs.Size();
    Resize(size);
    std::memcpy(buffer_, rhs.Data(), size);

    return *this;
}

ByteBuffer& ByteBuffer::operator=(ByteBuffer&& rhs)
{
    std::swap(buffer_, rhs.buffer_);
    std::swap(size_, rhs.size_);

    return *this;
}

std::uint64_t ByteBuffer::Size() const
{
    return size_;
}

void* ByteBuffer::Data() const
{
    return buffer_;
}

void ByteBuffer::Resize(std::uint64_t newSize)
{
    void* newBuffer = malloc(newSize);

    if (buffer_) {
        std::memcpy(newBuffer, buffer_, newSize);
        free(buffer_);
    }

    buffer_ = newBuffer;
    size_ = newSize;
}

ByteBuffer::~ByteBuffer()
{
    free(buffer_);
}