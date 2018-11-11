#pragma once

#include "BufferView.hpp"
#include "..\resources\Resource.hpp"

namespace VKW
{

struct ProvidedBuffer
{
public:
    ProvidedBuffer(BufferResourceHandle resource, std::uint32_t referenceCount = 0);

    BufferResourceHandle bufferResource_;
    std::uint32_t referenceCount_;
};

}