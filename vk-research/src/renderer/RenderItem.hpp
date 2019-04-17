#pragma once

#include <cstdint>
#include <limits>
#include "UniformBuffer.hpp"
#include "RendererDescriptorSetSlotsOwner.hpp"

namespace Render
{

// TODO
constexpr std::size_t RENDER_ITEM_UNIFORM_MAX_COUNT = 4;

struct RenderItem
{
    std::uint32_t vertexCount_;
    std::uint32_t descriptorSetCount_;
    // TODO
    DescriptorSetsOwner<RenderItem> descriptorSetSlots_;
};

struct RenderItemHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}