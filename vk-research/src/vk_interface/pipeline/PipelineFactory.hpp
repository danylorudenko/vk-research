#pragma once

#include "..\..\class_features\NonCopyable.hpp"

#include <vulkan\vulkan.h>
#include <vector>

#include "Pipeline.hpp"
#include "RenderPass.hpp"

namespace VKW
{

class ImportTable;
class Device;


struct PipelineFactoryDesc
{
    ImportTable* table_;
    Device* device_;
};


struct GraphicsPipelineDesc
{
    bool optimized_;

    // shader stages count
    // pShaderStages
    //
    // vertex input desc
    // primitive assembly desc
    // viewport desc
    // resterization desc
    // multisample desc
    // depth-stencil desc
    // color-blend state desc
    // 
    //
    // layout
    RenderPassHandle renderPass_;
    // subpass index
};

class PipelineFactory
{
public:
    PipelineFactory();
    PipelineFactory(PipelineFactoryDesc const& desc);

    PipelineFactory(PipelineFactory&& rhs);
    PipelineFactory& operator=(PipelineFactory&& rhs);

    PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc const& desc);
    void DestroyPipeline(PipelineHandle pipeline);
    
    Pipeline* GetPipeline(PipelineHandle handle) const;
    
    ~PipelineFactory();

private:
    ImportTable* table_;
    Device* device_;

    std::vector<Pipeline*> pipelines_;
};

}