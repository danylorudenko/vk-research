#pragma once

#include "..\class_features\NonCopyable.hpp"

#include <unordered_map>
#include <map>

#include "RootDef.hpp"
#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\buffer\BuffersProvider.hpp"
#include "..\vk_interface\image\ImagesProvider.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\pipeline\DescriptorLayoutController.hpp"
#include "..\vk_interface\pipeline\ShaderModuleFactory.hpp"
#include "..\vk_interface\pipeline\PipelineFactory.hpp"
#include "..\vk_interface\runtime\PresentationController.hpp"

#include "Pass.hpp"
#include "SetLayout.hpp"
#include "RendererPipeline.hpp"
#include "UniformBuffer.hpp"

namespace VKW
{
class ResourceRendererProxy;
class Worker;
class Loader;
}

namespace Render
{

struct RootPassDesc
{
    std::uint32_t colorAttachmentsCount_;
    ResourceKey colorAttachments_[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    ResourceKey depthStencilAttachment_;
};

struct RootDesc
{
    VKW::Loader* loader_;
    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::ImagesProvider* imagesProvider_;
    VKW::FramedDescriptorsHub* framedDescriptorsHub_;
    VKW::DescriptorLayoutController* layoutController_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::PresentationController* presentationController_;
    VKW::Worker* mainWorkerTemp_;
    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;
};

struct RootShaderDesc
{
    VKW::ShaderModuleDesc desc_;
};

struct RootPipelineDesc
{
    bool optimized_;

    std::uint32_t shaderStagesCount_;
    RootShaderDesc shaderStages_[VKW::Pipeline::MAX_SHADER_STAGES];

    VKW::InputAssemblyInfo* inputAssemblyInfo_;
    VKW::VertexInputInfo* vertexInputInfo_;
    VKW::ViewportInfo* viewportInfo_;
    VKW::DepthStencilInfo* depthStencilInfo_;
    // blending info should be here later

    VKW::PipelineLayoutDesc* layoutDesc_;
    RenderPassKey renderPass_;
};

class Root
    : public NonCopyable
{
public:
    static constexpr char const* SWAPCHAIN_IMAGE_KEY = "swapchain";


    using GlobalImagesMap = std::unordered_map<ResourceKey, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VKW::ProxyBufferHandle>;
    using UniformBufferMap = std::unordered_map< UniformBufferId, UniformBuffer >;

    using RenderPassMap = std::map<RenderPassKey, Pass>;
    using SetLayoutMap = std::map<SetLayoutKey, SetLayout>;
    using PipelineMap = std::map<PipelineKey, Pipeline>;

    using RenderGraphRoot = std::vector<RenderPassKey>;


    Root();
    Root(RootDesc const& desc);
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();


    UniformBufferId AcquireUniformBuffer(std::uint32_t size);
    UniformBuffer FindUniformBuffer(UniformBufferId id);
    VKW::BufferView* FindUniformBuffer(UniformBufferId id, std::uint32_t frame);
    void ReleaseUniformBuffer(UniformBufferId id);

    void DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc);
    VKW::ProxyBufferHandle FindGlobalBuffer(ResourceKey const& key);
    VKW::BufferView* FindGlobalBuffer(ResourceKey const& key, std::uint32_t frame);

    void DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc);
    VKW::ProxyImageHandle FindGlobalImage(ResourceKey const& key);
    VKW::ImageView* FindGlobalImage(ResourceKey const& key, std::uint32_t frame);

    void DefineRenderPass(RenderPassKey const& key, RootPassDesc const& desc);
    Pass& FindPass(RenderPassKey const& key);

    void DefineSetLayout(SetLayoutKey const& key, VKW::DescriptorSetLayoutDesc const& desc);
    SetLayout& FindSetLayout(SetLayoutKey const& key);

    void DefineGraphicsPipeline(PipelineKey const& key, RootPipelineDesc const& desc);
    Pipeline& FindPipeline(PipelineKey const& key);

    void PushPassTemp(RenderPassKey const& key);

    void IterateRenderGraph();

private:
    VKW::Loader* loader_;
    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::ImagesProvider* imagesProvider_;
    VKW::FramedDescriptorsHub* framedDescriptorsHub_;
    VKW::DescriptorLayoutController* layoutController_;
    VKW::ShaderModuleFactory* shaderModuleFactory_;
    VKW::PipelineFactory* pipelineFactory_;

    VKW::PresentationController* presentationController_;
    VKW::Worker* mainWorkerTemp_;

    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;
    UniformBufferMap uniformBuffers_;
    UniformBufferId nextUniformBufferId_;

    RenderPassMap renderPassMap_;
    SetLayoutMap setLayoutMap_;
    PipelineMap pipelineMap_;


    RenderGraphRoot renderGraphRootTemp_;

};

}