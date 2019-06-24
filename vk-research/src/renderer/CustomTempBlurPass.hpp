#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "Pass.hpp"
#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\VkInterfaceConstants.hpp"

class IOManager;

namespace VKW
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

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;
    VKW::Swapchain* swapchain_;

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
    IOManager* ioManager_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;
    VKW::Swapchain* swapchain_;

    ResourceKey sceneColorBuffer_;

    ResourceKey horizontalBlurBuffer_;
    ResourceKey verticalBlurBuffer_;
    ResourceKey blurMaskTexture_;

    PipelineKey horizontalBlurPipeline_;
    PipelineKey verticalBlurPipeline_;
    PipelineKey mixPipeline_;

    SetLayoutKey universalSetLayout_;
    SetLayoutKey mixSetLayout_;

    VKW::ProxySetHandle horizontalDescriptorSet_;
    VKW::ProxySetHandle verticalDescriptorSet_;
    VKW::ProxySetHandle mixDescriptorSet_;

    UniformBufferHandle mixFactorUniformBuffer_;
};

}

