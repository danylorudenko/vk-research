#pragma once

#include <cstdint>
#include "UniformBuffer.hpp"
#include "RootDef.hpp"

namespace Render
{

constexpr std::size_t UNIFORM_ID_MAX_LENGTH = 64;
constexpr std::size_t RENDER_ITEM_UNIFORM_MAX_COUNT = 4;

class Root;

struct RenderItemUniform
{
    RenderItemUniform();
    RenderItemUniform(Root* root, char const* name, std::uint32_t dataSize, std::uint8_t const* dataSource);

    template<typename T>
    RenderItemUniform(Root* root, char const* name, T* sourceData)
        : RenderItemUniform{ root, name, sizeof(sourceData), reinterpret_cast<std::uint8_t const*>(sourceData) }
    {
    }

    RenderItemUniform(RenderItemUniform const& rhs) = delete;
    RenderItemUniform(RenderItemUniform&& rhs);
    RenderItemUniform& operator=(RenderItemUniform const& rhs) = delete;
    RenderItemUniform& operator=(RenderItemUniform&& rhs);

    ~RenderItemUniform();




    char name_[UNIFORM_ID_MAX_LENGTH];
    std::uint32_t hostBufferSize_;
    std::uint8_t* hostBuffer_;
    UniformBufferHandle serverBufferHandle_;
};

struct RenderItem
{
    std::uint32_t uniformsCount_;
    RenderItemUniform uniforms_[RENDER_ITEM_UNIFORM_MAX_COUNT];
};

}