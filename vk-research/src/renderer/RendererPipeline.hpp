#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\Pipeline.hpp"
#include "..\vk_interface\pipeline\DescriptorLayout.hpp"
#include "..\renderer\RootDef.hpp"

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

class Pipeline
    : public NonCopyable
{
public:
    Pipeline();
    Pipeline(VKW::PipelineHandle handle);

    Pipeline(Pipeline&& rhs);
    Pipeline& operator=(Pipeline&& rhs);

    ~Pipeline();

private:
    VKW::PipelineHandle pipelineHandle_;
    VKW::PipelineLayoutHandle layoutHandle_;

    PipelineProperties properties_;
};

}