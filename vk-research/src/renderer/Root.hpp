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
#include "..\vk_interface\pipeline\PipelineFactory.hpp"
#include "..\vk_interface\runtime\PresentationController.hpp"

#include "Pass.hpp"
#include "SetLayout.hpp"
#include "RendererPipeline.hpp"

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
    VKW::PipelineFactory* pipelineFactory_;
    VKW::PresentationController* presentationController_;
    VKW::Worker* mainWorkerTemp_;
    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;
};

struct RootPipelineDesc
{
    bool optimized_;

    std::uint32_t shaderStagesCount_;
    VKW::ShaderStageInfo shaderStages_[VKW::Pipeline::MAX_SHADER_STAGES];

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
    using GlobalImagesMap = std::unordered_map<ResourceKey, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VKW::ProxyBufferHandle>;
    using RenderPassMap = std::map<RenderPassKey, Pass>;
    using SetLayoutMap = std::map<SetLayoutKey, SetLayout>;
    using PipelineMap = std::map<PipelineKey, Pipeline>;

    using RenderGraphRoot = std::vector<RenderPassKey>;

    Root();
    Root(RootDesc const& desc);
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();


    void DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc);
    VKW::ProxyBufferHandle FindGlobalBuffer(ResourceKey const& key);

    void DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc);
    VKW::ProxyImageHandle FindGlobalImage(ResourceKey const& key);

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
    VKW::PipelineFactory* pipelineFactory_;

    VKW::PresentationController* presentationController_;
    VKW::Worker* mainWorkerTemp_;

    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;

    RenderPassMap renderPassMap_;
    SetLayoutMap setLayoutMap_;
    PipelineMap pipelineMap_;


    RenderGraphRoot renderGraphRootTemp_;

};

}