#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

enum BufferUsage
{
    VERTEX_INDEX,
    UNIFORM,
    UPLOAD_BUFFER
};

struct Buffer
{
    VkBuffer handle_;
};

struct BufferCreateInfo
{
    std::uint64_t size_;
    std::uint64_t alignment_;
    BufferUsage usage_;
};