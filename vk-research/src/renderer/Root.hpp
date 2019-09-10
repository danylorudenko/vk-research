#pragma once

#include "..\class_features\NonCopyable.hpp"

#include <unordered_map>
#include <map>
#include <memory>

#include "RootDef.hpp"
#include "..\VAL\ProxyHandles.hpp"
#include "..\VAL\buffer\BuffersProvider.hpp"
#include "..\VAL\image\ImagesProvider.hpp"
#include "..\VAL\pipeline\RenderPassController.hpp"
#include "..\VAL\pipeline\DescriptorLayoutController.hpp"
#include "..\VAL\pipeline\ShaderModuleFactory.hpp"
#include "..\VAL\pipeline\PipelineFactory.hpp"
#include "..\VAL\runtime\PresentationController.hpp"

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
        VAL::RenderPassAttachmentUsage usage_;
    }
    colorAttachments_[VAL::RenderPass::MAX_COLOR_ATTACHMENTS];
    ResourceKey depthStencilAttachment_;
};

struct RootComputePassDesc
{

};

struct RootDesc
{
    VAL::Loader* loader_;
    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::RenderPassController* renderPassController_;
    VAL::ImagesProvider* imagesProvider_;
    VAL::FramedDescriptorsHub* framedDescriptorsHub_;
    VAL::DescriptorLayoutController* layoutController_;
    VAL::ShaderModuleFactory* shaderModuleFactory_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::PresentationController* presentationController_;
    VAL::Worker* mainWorkerTemp_;
};

struct ShaderDesc
{
    VAL::ShaderModuleType type_;
    std::string relativePath_;
};

struct PipelineLayoutDesc
{
    std::uint32_t staticMembersCount_;
    SetLayoutKey staticMembers_[VAL::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    std::uint32_t instancedMembersCount_;
    SetLayoutKey instancedMembers_[VAL::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
};


struct GraphicsPipelineDesc
{
    bool optimized_;

    std::uint32_t shaderStagesCount_;
    ShaderKey shaderStages_[VAL::Pipeline::MAX_SHADER_STAGES];

    VAL::InputAssemblyInfo* inputAssemblyInfo_;
    VAL::VertexInputInfo* vertexInputInfo_;
    VAL::ViewportInfo* viewportInfo_;
    VAL::DepthStencilInfo* depthStencilInfo_;
    std::uint32_t dynamicStateFlags_;  // VAL::PipelineDynamicStateFlags
    std::uint32_t blendingState_; // VAL::PipelineBlendingState

    PipelineLayoutDesc* layoutDesc_;
    PassKey renderPass_;
};

struct ComputePipelineDesc
{
    bool optimized_;
    VAL::ShaderStageInfo shaderStage_;
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
    members_[VAL::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};

struct MaterialDesc
{
    MaterialTemplateKey templateKey_;
    struct PerPassData
    {
        SetOwnerDesc setOwnerDesc_[VAL::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
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

    using GlobalImagesMap = std::unordered_map<ResourceKey, VAL::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VAL::ProxyBufferHandle>;
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

    VAL::ImportTable* VulkanFuncTable() const;

    UniformBufferHandle AcquireUniformBuffer(std::uint32_t size);
    UniformBuffer& FindUniformBuffer(UniformBufferHandle id);
    VAL::BufferView* FindUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void ReleaseUniformBuffer(UniformBufferHandle id);
    void* MapUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void UnmapUniformBuffer(UniformBufferHandle id, std::uint32_t frame);
    void FlushUniformBuffer(UniformBufferHandle id, std::uint32_t frame);

    void* MapBuffer(ResourceKey const& key, std::uint32_t frame);
    void FlushBuffer(ResourceKey const& key, std::uint32_t frame);
    void* MapImage(ResourceKey const& key, std::uint32_t frame);
    void FlushImage(ResourceKey const& key, std::uint32_t frame);


    VAL::BufferResource* GetViewResource(VAL::BufferView* view);
    VAL::MemoryRegion* GetViewMemory(VAL::BufferView* view);
    VAL::MemoryPage* GetViewMemoryPage(VAL::BufferView* view);
    VAL::ImageResource* GetViewResource(VAL::ImageView* view);
    VAL::MemoryRegion* GetViewMemory(VAL::ImageView* view);
    VAL::MemoryPage* GetViewMemoryPage(VAL::ImageView* view);

    void DefineGlobalBuffer(ResourceKey const& key, VAL::BufferViewDesc const& desc);
    VAL::ProxyBufferHandle FindGlobalBuffer(ResourceKey const& key);
    VAL::BufferView* FindGlobalBuffer(ResourceKey const& key, std::uint32_t frame);

    ResourceKey GetDefaultSceneColorOutput() const;
    ResourceKey GetSwapchain() const;
    VkSampler GetDefaultSampler() const;
    std::uint32_t GetDefaultSceneColorBufferThreeshold() const;
    void DefineGlobalImage(ResourceKey const& key, VAL::ImageViewDesc const& desc);
    VAL::ProxyImageHandle FindGlobalImage(ResourceKey const& key);
    VAL::ImageView* FindGlobalImage(ResourceKey const& key, std::uint32_t frame);

    void DefineRenderPass(PassKey const& key, RootGraphicsPassDesc const& desc);
    void DefineCustomBlurPass(PassKey const& key, ResourceKey const& sceneColorBuffer, IOManager* ioManager);
    BasePass& FindPass(PassKey const& key);

    void DefineSetLayout(SetLayoutKey const& key, VAL::DescriptorSetLayoutDesc const& desc);
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


    VAL::ResourceRendererProxy* ResourceProxy() const;

    void PushPass(PassKey const& key);

    

    VAL::PresentationContext AcquireNextPresentationContext();
    VAL::WorkerFrameCommandReciever BeginRenderGraph(VAL::PresentationContext const& presentationContext);
    void IterateRenderGraph(VAL::PresentationContext const& presentationContext, VAL::WorkerFrameCommandReciever& commandReciever);
    void EndRenderGraph(VAL::PresentationContext const& presentationContext, VAL::WorkerFrameCommandReciever& commandReciever);


private:
    /*DESCRIPTOR_TYPE_TEXTURE,
        DESCRIPTOR_TYPE_SAMPLER,
        DESCRIPTOR_TYPE_UNIFORM_BUFFER*/

    void Decorate_VKWProxyDescriptorWriteDesc_UniformBuffer(VAL::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, UniformBufferHandle bufferHandle);
    void Decorate_VKWProxyDescriptorWriteDesc_Texture(VAL::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, VAL::ProxyImageHandle imageView);

    void InitializeSetsOwner(DescriptorSetsOwner& owner, std::uint32_t setsCount, SetLayoutKey const* setLayoutKeys, SetOwnerDesc const* setOwnerDescs);


private:
    static constexpr char const* SWAPCHAIN_IMAGE_KEY = "swapchain";
    static constexpr char const* SCENE_COLOR_OUTPUT_KEY = "colf";
    static constexpr std::uint32_t COLOR_BUFFER_THREESHOLD = 10;


private:
    VAL::Loader* loader_;
    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::RenderPassController* renderPassController_;
    VAL::ImagesProvider* imagesProvider_;
    VAL::FramedDescriptorsHub* framedDescriptorsHub_;
    VAL::DescriptorLayoutController* layoutController_;
    VAL::ShaderModuleFactory* shaderModuleFactory_;
    VAL::PipelineFactory* pipelineFactory_;

    VAL::PresentationController* presentationController_;
    // TODO
    VAL::Worker* mainWorkerTemp_;

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