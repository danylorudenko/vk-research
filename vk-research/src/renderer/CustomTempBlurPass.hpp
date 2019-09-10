#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "Pass.hpp"
#include "..\VAL\ProxyHandles.hpp"
#include "..\VAL\VkInterfaceConstants.hpp"

class IOManager;


namespace VAL
{
    class ImportTable;
    class Device;
    class ResourceRendererProxy;
    class ShaderModuleFactory;
    class PipelineFactory;
    class DescriptorLayoutController;
    class Swapchain;
}

namespace Render
{

class Root;

struct CustomTempBlurPassDesc
{
    Root* root_;
    IOManager* ioManager_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::ShaderModuleFactory* shaderModuleFactory_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;
    VAL::Swapchain* swapchain_;

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

    virtual void Begin(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Apply(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;


private:
    Root* root_;
    IOManager* ioManager_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::ShaderModuleFactory* shaderModuleFactory_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;
    VAL::Swapchain* swapchain_;

    ResourceKey sceneColorBuffer_;

    ResourceKey blurBuffer_;
    ResourceKey blurMaskTexture_;

    PipelineKey blurFastPipeline_;
    PipelineKey blurFullPipeline_;

    SetLayoutKey blurSetLayout_;

    VAL::ProxySetHandle blurDescriptorSet_;

    UniformBufferHandle mixFactorUniformBuffer_;
};

}

