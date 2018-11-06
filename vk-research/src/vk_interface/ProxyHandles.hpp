#pragma once

#include <cstdint>
#include <limits>

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

}