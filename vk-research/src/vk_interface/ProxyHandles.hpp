#pragma once

#include <cstdint>
#include <limits>
#include "image\ImageView.hpp"
#include "buffer\BufferView.hpp"
#include "runtime\FramedDescriptorsHub.hpp"

namespace VKW
{

struct ProxySetHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct ProxyFramebufferHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct ProxyBufferHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

struct ProxyImageHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}