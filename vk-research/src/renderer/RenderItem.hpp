#pragma once

#include <cstdint>
#include <limits>
#include "UniformBuffer.hpp"
#include "RendererDescriptorSet.hpp"

namespace Render
{

// TODO
constexpr std::size_t RENDER_ITEM_UNIFORM_MAX_COUNT = 4;

struct RenderItem
{
    std::uint32_t vertexCount_;
    std::uint32_t descriptorSetCount_;
    // TODO
    DescriptorSet descriptorSets_[RENDER_ITEM_UNIFORM_MAX_COUNT];
};

struct RenderItemHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}