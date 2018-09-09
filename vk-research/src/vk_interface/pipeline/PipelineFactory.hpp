#pragma once

#include "../../class_features/NonCopyable.hpp"

#include <vulkan/vulkan.h>
#include <vector>

#include "Pipeline.hpp"

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

};

class PipelineFactory
{
    PipelineFactory();
    PipelineFactory(PipelineFactoryDesc const& desc);

    PipelineFactory(PipelineFactory&& rhs);
    PipelineFactory& operator=(PipelineFactory&& rhs);

    Pipeline* CreateGraphicsPipeline(GraphicsPipelineDesc const& desc);
    void DestroyPipeline(Pipeline* pipeline);

    ~PipelineFactory();

private:
    ImportTable* table_;
    Device* device_;

    std::vector<Pipeline> pipelines_;
};

}