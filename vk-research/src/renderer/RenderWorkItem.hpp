#pragma once

#include <cstdint>
#include <limits>
#include "UniformBuffer.hpp"
#include "RendererDescriptorSetsOwner.hpp"
#include "RootDef.hpp"

namespace Render
{

struct RenderWorkItem
{
    ResourceKey vertexBufferKey_;
    ResourceKey indexBufferKey_;
    std::uint32_t vertexCount_;
    std::uint32_t vertexBindOffset_;
    std::uint32_t indexCount_;
    std::uint32_t indexBindOffset_;
    DescriptorSetsOwner descriptorSetsOwner_;
};

struct RenderWorkItemHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}