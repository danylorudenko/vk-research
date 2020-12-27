#pragma once

#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\resources\Resource.hpp>

namespace VKW
{

struct ProvidedBuffer
{
public:
    ProvidedBuffer(BufferResource* resource, std::uint32_t referenceCount = 0);

    BufferResource* bufferResource_;
    std::uint32_t referenceCount_;
};

}