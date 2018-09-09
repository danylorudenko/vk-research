#pragma once

#include <cstdint>

class ByteBuffer
{
public:
    ByteBuffer();
    ByteBuffer(std::uint64_t size);
    ByteBuffer(ByteBuffer const& rhs);
    ByteBuffer(ByteBuffer&& rhs);

    ByteBuffer& operator=(ByteBuffer const& rhs);
    ByteBuffer& operator=(ByteBuffer&& rhs);

    std::uint64_t Size() const;

    void Resize(std::uint64_t newSize);
    void* Data() const;

    template<typename T>
    T As()
    {
        return reinterpret_cast<T>(buffer);
    }

    ~ByteBuffer();

private:
    void* buffer_;
    std::uint64_t size_;
};