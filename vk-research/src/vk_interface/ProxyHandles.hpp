#pragma once

#include <cstdint>
#include <limits>

#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\runtime\FramedDescriptorsHub.hpp>

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