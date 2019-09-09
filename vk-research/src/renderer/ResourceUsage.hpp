#pragma once

#include <cstdint>

namespace Render
{
    using ResourceUsage = std::uint64_t;

    enum ResourceUsageBits
    {
        RESOURCE_USAGE_BUFFER_SHADER_READ_BIT                   = 1 << 0,
        RESOURCE_USAGE_BUFFER_SHADER_WRITE_BIT                  = 1 << 1,
        RESOURCE_USAGE_BUFFER_SHADER_UNIFORM_BIT                = 1 << 2,

        RESOURCE_USAGE_IMAGE_TEXTURE_READ_BIT                   = 1 << 3,
        RESOURCE_USAGE_IMAGE_TEXTURE_WRITE_BIT                  = 1 << 4,
        RESOURCE_USAGE_IMAGE_COLOR_ATTACHMENT_BIT               = 1 << 5,
        RESOURCE_USAGE_IMAGE_COLOR_ATTACHMENT_SWAPCHAIN_BIT     = 1 << 6
    };

}

