#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\Pipeline.hpp"
#include "..\vk_interface\pipeline\DescriptorLayout.hpp"
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
    std::uint32_t pipelineDynamicStateFlags_; // VKW::PipelineDynamicStatesFlags
    VertexStageProperties vertexProperties_;
    FragmentStageProperties fragmentProperties_;
};

struct Pipeline
{
    std::uint32_t staticLayoutMembersCount_;
    SetLayoutKey staticLayoutKeys_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    std::uint32_t instancedLayoutMembersCount_;
    SetLayoutKey instancedLayoutKeys_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    VKW::PipelineLayoutHandle layoutHandle_;

    VKW::PipelineHandle pipelineHandle_;
    PipelineProperties properties_;

    std::vector<RenderWorkItem> renderItems_;
};

}