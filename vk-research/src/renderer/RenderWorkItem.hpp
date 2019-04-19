#pragma once

#include <cstdint>
#include <limits>
#include "UniformBuffer.hpp"
#include "RendererDescriptorSetsOwner.hpp"

namespace Render
{

struct RenderWorkItem
{
    std::uint32_t vertexCount_;
    DescriptorSetsOwner descriptorSetsOwner_;
};

struct RenderWorkItemHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}