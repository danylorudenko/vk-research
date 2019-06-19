#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "Pass.hpp"

namespace VKW
{
    class ImportTable;
    class Device;
    class ResourceRendererProxy;
    class ShaderModuleFactory;
    class PipelineFactory;
    class DescriptorLayoutController;
}

namespace Render
{

class Root;

struct CustomTempBlurPassDesc
{
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;

    ResourceKey sceneColorBuffer_;
};

class CustomTempBlurPass
    : public BasePass
{
public:
    CustomTempBlurPass();
    CustomTempBlurPass(CustomTempBlurPassDesc const& desc);
    CustomTempBlurPass(CustomTempBlurPass&& rhs);
    CustomTempBlurPass& operator=(CustomTempBlurPass&& rhs);
    ~CustomTempBlurPass();

    virtual void Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Apply(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;


private:
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;

    ResourceKey sceneColorBuffer_;

    ResourceKey horizontalBlurBuffer_;
    ResourceKey verticalBlurBuffer_;

    PipelineKey horizontalBlurPipeline_;
    PipelineKey verticalBlurPipeline_;

    SetLayoutKey universalSetLayout_;
};

}

