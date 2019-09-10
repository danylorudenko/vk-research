#pragma once

#include "..\class_features\NonCopyable.hpp"

#include <unordered_map>
#include <map>
#include <memory>

#include "RootDef.hpp"
#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\buffer\BuffersProvider.hpp"
#include "..\vk_interface\image\ImagesProvider.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\pipeline\DescriptorLayoutController.hpp"
#include "..\vk_interface\pipeline\ShaderModuleFactory.hpp"
#include "..\vk_interface\pipeline\PipelineFactory.hpp"
#include "..\vk_interface\runtime\PresentationController.hpp"

#include "Shader.hpp"
#include "MaterialTemplate.hpp"
#include "Material.hpp"
#include "Pass.hpp"
#include "SetLayout.hpp"
#include "RendererPipeline.hpp"
#include "UniformBuffer.hpp"
#include "RenderWorkItem.hpp"

class IOManager;

namespace VAL
{
class ResourceRendererProxy;
class Worker;
class Loader;
}

namespace Render
{

struct RootGraphicsPassDesc
{
    std::uint32_t colorAttachmentsCount_;
    struct ColorAttachment
    {
        ResourceKey resourceKey_;
        VKW::RenderPassAttachmentUsage usage_;
    }
    colorAttachments_[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    ResourceKey depthStencilAttachment_;
};

struct RootComputePassDesc
{

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
};

struct ShaderDesc
{
    VKW::ShaderModuleType type_;
    std::string relativePath_;
};

struct PipelineLayoutDesc
{
    std::uint32_t staticMembersCount_;
    SetLayoutKey staticMembers_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    std::uint32_t instancedMembersCount_;
    SetLayoutKey instancedMembers_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
};


struct GraphicsPipelineDesc
{
    bool optimized_;

    std::uint32_t shaderStagesCount_;
    ShaderKey shaderStages_[VKW::Pipeline::MAX_SHADER_STAGES];

    VKW::InputAssemblyInfo* inputAssemblyInfo_;
    VKW::VertexInputInfo* vertexInputInfo_;
    VKW::ViewportInfo* viewportInfo_;
    VKW::DepthStencilInfo* depthStencilInfo_;
    std::uint32_t dynamicStateFlags_;  // VKW::PipelineDynamicStateFlags
    std::uint32_t blendingState_; // VKW::PipelineBlendingState

    PipelineLayoutDesc* layoutDesc_;
    PassKey renderPass_;
};

struct ComputePipelineDesc
{
    bool optimized_;
    VKW::ShaderStageInfo shaderStage_;
    PipelineLayoutDesc* layoutDesc_;
};


struct MaterialTemplateDesc
{
    std::uint32_t perPassDataCount_;
    struct PerPassData 
    {
        PassKey passKey_;
        PipelineKey pipelineKey_;
    } 
    perPassData_[MATERIAL_TEMPLATE_PASS_LIMIT];
};

struct UniformBufferSetMemberData
{
    std::uint32_t size_;
};

struct Texture2DSetMemberData
{
    ResourceKey imageKey_;
};

struct StorageBufferSetMemberData
{
};

struct SetOwnerDesc
{
    struct Member
    {
        UniformBufferSetMemberData uniformBuffer_;
        Texture2DSetMemberData texture2D_;
        StorageBufferSetMemberData storageBuffer_;
    }
    members_[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};

struct MaterialDesc
{
    MaterialTemplateKey templateKey_;
    struct PerPassData
    {
        SetOwnerDesc setOwnerDesc_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    } 
    perPassData_[MATERIAL_TEMPLATE_PASS_LIMIT];
};

struct RenderWorkItemDesc
{
    ResourceKey vertexBufferKey_;
    ResourceKey indexBufferKey_;
    std::uint32_t vertexCount_;
    std::uint32_t vertexBindOffset_;
    std::uint32_t indexCount_;
    std::uint32_t indexBindOffset_;
    SetOwnerDesc setOwnerDescs_[SCOPE_MAX_SETS_RENDERITEM];
};

class Root
    : public NonCopyable
{
public:

    using GlobalImagesMap = std::unordered_map<ResourceKey, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VKW::ProxyBufferHandle>;
    using UniformBufferMap = std::unordered_map< std::uint64_t, UniformBuffer >;

    using ShaderMap = std::map<ShaderKey, Shader>;
    using RenderPassMap = std::map<PassKey, std::unique_ptr<BasePass>>;
    using SetLayoutMap = std::map<SetLayoutKey, SetLayout>;
    using PipelineMap = std::map<PipelineKey, Pipeline>;
    using MaterialTemplateMap = std::map<MaterialTemplateKey, MaterialTemplate>;
    using MaterialMap = std::map<MaterialKey, Material>;

    using RenderGraphRoot = std::vector<PassKey>;


    Root();
    Root(RootDesc const& desc);
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();

    VKW::ImportTable* VulkanFuncTable() const;

    UniformBufferHandle AcquireUniformBuffer(std::uint32_t size);
    UniformBuffer& FindUniformBuffer(UniformBufferHandle id);
    VKW::BufferView* FindUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void ReleaseUniformBuffer(UniformBufferHandle id);
    void* MapUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void UnmapUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void FlushUniformBuffer(UniformBufferHandle id, std::uint32_t frame);

    void* MapBuffer(ResourceKey const& key, std::uint32_t frame);
    void FlushBuffer(ResourceKey const& key, std::uint32_t frame);
    void* MapImage(ResourceKey const& key, std::uint32_t frame);
    void FlushImage(ResourceKey const& key, std::uint32_t frame);


    VKW::BufferResource* GetViewResource(VKW::BufferView* view);
    VKW::MemoryRegion* GetViewMemory(VKW::BufferView* view);
    VKW::MemoryPage* GetViewMemoryPage(VKW::BufferView* view);
    VKW::ImageResource* GetViewResource(VKW::ImageView* view);
    VKW::MemoryRegion* GetViewMemory(VKW::ImageView* view);
    VKW::MemoryPage* GetViewMemoryPage(VKW::ImageView* view);

    void DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc);
    VKW::ProxyBufferHandle FindGlobalBuffer(ResourceKey const& key);
    VKW::BufferView* FindGlobalBuffer(ResourceKey const& key, std::uint32_t frame);

    ResourceKey GetDefaultSceneColorOutput() const;
    ResourceKey GetSwapchain() const;
    VkSampler GetDefaultSampler() const;
    std::uint32_t GetDefaultSceneColorBufferThreeshold() const;
    void DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc);
    VKW::ProxyImageHandle FindGlobalImage(ResourceKey const& key);
    VKW::ImageView* FindGlobalImage(ResourceKey const& key, std::uint32_t frame);

    void DefineRenderPass(PassKey const& key, RootGraphicsPassDesc const& desc);
    void DefineCustomBlurPass(PassKey const& key, ResourceKey const& sceneColorBuffer, IOManager* ioManager);
    BasePass& FindPass(PassKey const& key);

    void DefineSetLayout(SetLayoutKey const& key, VKW::DescriptorSetLayoutDesc const& desc);
    SetLayout& FindSetLayout(SetLayoutKey const& key);

    void DefineShader(ShaderKey const& key, ShaderDesc const& desc);
    Shader& FindShader(ShaderKey const& key);

    void DefineGraphicsPipeline(PipelineKey const& key, GraphicsPipelineDesc const& desc);
    void DefineComputePipeline(PipelineKey const& key, ComputePipelineDesc const& desc);
    Pipeline& FindPipeline(PipelineKey const& key);

    void DefineMaterialTemplate(MaterialTemplateKey const& key, MaterialTemplateDesc const& desc);
    MaterialTemplate& FindMaterialTemplate(MaterialTemplateKey const& key);

    void DefineMaterial(MaterialKey const& key, MaterialDesc const& desc);
    Material& FindMaterial(MaterialKey const& key);
    void RegisterMaterial(MaterialKey const& key);


    RenderWorkItemHandle ConstructRenderWorkItem(PipelineKey const& key, RenderWorkItemDesc const& desc);
    RenderWorkItemHandle ConstructRenderWorkItem(Pipeline& pipeline, RenderWorkItemDesc const& desc);
    RenderWorkItem* FindRenderWorkItem(PipelineKey const& key, RenderWorkItemHandle handle);
    RenderWorkItem* FindRenderWorkItem(Pipeline& pipeline, RenderWorkItemHandle handle);
    void ReleaseRenderWorkItem(PipelineKey const& key, RenderWorkItemHandle handle);
    void ReleaseRenderWorkItem(Pipeline& pipeline, RenderWorkItemHandle handle);

    void ImageLayoutTransition(std::uint32_t context, std::uint32_t imagesCount, VkImage* images, VkImageLayout* targetLayouts);
    void CopyStagingBufferToGPUBuffer(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context);
    void CopyStagingBufferToGPUTexture(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context);
    void BlitImages(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context, VkImageLayout dstEndLayout, VkAccessFlags dstEndAccessFlags);


    VKW::ResourceRendererProxy* ResourceProxy() const;

    void PushPass(PassKey const& key);

    

    VKW::PresentationContext AcquireNextPresentationContext();
    VKW::WorkerFrameCommandReciever BeginRenderGraph(VKW::PresentationContext const& presentationContext);
    void IterateRenderGraph(VKW::PresentationContext const& presentationContext, VKW::WorkerFrameCommandReciever& commandReciever);
    void EndRenderGraph(VKW::PresentationContext const& presentationContext, VKW::WorkerFrameCommandReciever& commandReciever);


private:
    /*DESCRIPTOR_TYPE_TEXTURE,
        DESCRIPTOR_TYPE_SAMPLER,
        DESCRIPTOR_TYPE_UNIFORM_BUFFER*/

    void Decorate_VKWProxyDescriptorWriteDesc_UniformBuffer(VKW::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, UniformBufferHandle bufferHandle);
    void Decorate_VKWProxyDescriptorWriteDesc_Texture(VKW::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, VKW::ProxyImageHandle imageView);

    void InitializeSetsOwner(DescriptorSetsOwner& owner, std::uint32_t setsCount, SetLayoutKey const* setLayoutKeys, SetOwnerDesc const* setOwnerDescs);


private:
    static constexpr char const* SWAPCHAIN_IMAGE_KEY = "swapchain";
    static constexpr char const* SCENE_COLOR_OUTPUT_KEY = "colf";
    static constexpr std::uint32_t COLOR_BUFFER_THREESHOLD = 10;


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
    // TODO
    VKW::Worker* mainWorkerTemp_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;
    UniformBufferMap uniformBuffers_;
    std::uint64_t nextUniformBufferId_;

    ShaderMap shaderMap_;

    RenderPassMap renderPassMap_;
    SetLayoutMap setLayoutMap_;
    PipelineMap pipelineMap_;

    MaterialTemplateMap materialTemplateMap_;
    MaterialMap materialMap_;


    RenderGraphRoot renderGraphRootTemp_;

};

}