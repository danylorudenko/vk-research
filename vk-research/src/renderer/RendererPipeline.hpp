#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\VAL\pipeline\Pipeline.hpp"
#include "..\VAL\pipeline\DescriptorLayout.hpp"
#include "RootDef.hpp"
#include "RenderWorkItem.hpp"

namespace Render
{

struct VertexStageProperties
{
    //ShaderKey shader_;
};

struct FragmentStageProperties
{
    //ShaderKey shader_;
};

struct PipelineProperties
{
    std::uint32_t pipelineDynamicStateFlags_; // VAL::PipelineDynamicStatesFlags
    VertexStageProperties vertexProperties_;
    FragmentStageProperties fragmentProperties_;
};

struct Pipeline
{
    std::uint32_t staticLayoutMembersCount_;
    SetLayoutKey staticLayoutKeys_[VAL::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    std::uint32_t instancedLayoutMembersCount_;
    SetLayoutKey instancedLayoutKeys_[VAL::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    VAL::PipelineLayoutHandle layoutHandle_;

    VAL::PipelineHandle pipelineHandle_;
    PipelineProperties properties_;

    std::vector<RenderWorkItem> renderItems_;
};

}