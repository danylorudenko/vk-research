#pragma once

#include <cstdint>
#include <limits>
#include "UniformBuffer.hpp"
#include "RootDef.hpp"

namespace Render
{

constexpr std::size_t UNIFORM_ID_MAX_LENGTH = 64;
constexpr std::size_t RENDER_ITEM_UNIFORM_MAX_COUNT = 4;

struct RenderItemUniform
{
    char name_[UNIFORM_ID_MAX_LENGTH];
    std::uint32_t hostBufferSize_;
    std::uint8_t* hostBuffer_;
    UniformBufferHandle serverBufferHandle_;
};

struct RenderItem
{
    std::uint32_t uniformBuffersCount_;
    RenderItemUniform uniformBuffers_[RENDER_ITEM_UNIFORM_MAX_COUNT];
};

struct RenderItemHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

}