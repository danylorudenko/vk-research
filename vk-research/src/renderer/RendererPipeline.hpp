#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\Pipeline.hpp"
#include "..\vk_interface\pipeline\DescriptorLayout.hpp"
#include "RootDef.hpp"
#include "RenderItem.hpp"

namespace Render
{

struct VertexStageProperties
{
    ShaderKey shader_;
};

struct FragmentStageProperties
{
    ShaderKey shader_;
};

struct PipelineProperties
{
    VertexStageProperties vertexProperties_;
    FragmentStageProperties fragmentProperties_;
};

struct Pipeline
{
    VKW::PipelineHandle pipelineHandle_;
    VKW::PipelineLayoutHandle layoutHandle_;

    PipelineProperties properties_;
    std::vector<RenderItem> renderItems_;
};

}