#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\VAL\pipeline\RenderPassController.hpp"
#include "..\VAL\ResourceRendererProxy.hpp"
#include "Material.hpp"
#include "RootDef.hpp"

namespace VAL
{
class ImportTable;
class Device;
class ResourceRendererProxy;
class RenderPassController;
class PipelineFactory;
class DescriptorLayoutController;
class Worker;
struct WorkerFrameCommandReciever;
struct FramedDescriptorsHub;
struct RenderPassDesc;
}

namespace Render
{

class Root;


class BasePass
    : public NonCopyable
{
public:
    virtual void Begin(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) = 0;
    virtual void Apply(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) = 0;
    virtual void End(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) = 0;
};


struct GraphicsPassDesc
{
    Root* root_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* proxy_;
    VAL::RenderPassController* renderPassController_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;
    VAL::FramedDescriptorsHub* framedDescriptorsHub_;
    VAL::ImagesProvider* imagesProvider_;

    std::uint32_t width_;
    std::uint32_t height_;

    std::uint32_t colorAttachmentCount_;
    struct ColorAttachment
    {
        VAL::ProxyImageHandle handle_;
        VAL::RenderPassAttachmentUsage usage_;
    }
    colorAttachments_[VAL::RenderPass::MAX_COLOR_ATTACHMENTS];
    VAL::ProxyImageHandle* depthStencilAttachment_;
    
};

class GraphicsPass
    : public BasePass
{
public:
    GraphicsPass();
    GraphicsPass(GraphicsPassDesc const& desc);
    GraphicsPass(GraphicsPass&& rhs);
    GraphicsPass& operator=(GraphicsPass&& rhs);
    ~GraphicsPass();

    VAL::RenderPassHandle VKWRenderPass() const;

    virtual void Begin(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Apply(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;

    void RegisterMaterialData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey);

private:
    struct MaterialDelegatedData
    {
        MaterialDelegatedData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey)
            : materialKey_{ materialKey }, materialPerPassDataId_{ materialPerPassDataId }, pipelineKey_{ pipelineKey } { }

        MaterialKey materialKey_;
        std::uint32_t materialPerPassDataId_;

        PipelineKey pipelineKey_;
    };


    Root* root_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::RenderPassController* renderPassController_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;

    VAL::RenderPassHandle vkRenderPass_;
    VAL::ProxyFramebufferHandle framebuffer_;
    std::uint32_t width_;
    std::uint32_t height_;

    std::vector<MaterialDelegatedData> materialDelegatedData_; 
};


struct ComputePassDesc
{
    Root* root_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* proxy_;
    VAL::RenderPassController* renderPassController_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;
    VAL::FramedDescriptorsHub* framedDescriptorsHub_;
    VAL::ImagesProvider* imagesProvider_;
};

enum ComputePassResourceUsage
{
    COMPUTE_PASS_RESOURCE_USAGE_BUFFER_READ,
    COMPUTE_PASS_RESOURCE_USAGE_BUFFER_WRITE,
    COMPUTE_PASS_RESOURCE_USAGE_BUFFER_READ_AFTER_WRITE,
    COMPUTE_PASS_RESOURCE_USAGE_IMAGE_READ,
    COMPUTE_PASS_RESOURCE_USAGE_IMAGE_WRITE,
    COMPUTE_PASS_RESOURCE_USAGE_IMAGE_READ_AFTER_WRITE
};

struct ComputePass
    : public BasePass
{
public:
    ComputePass();
    ComputePass(ComputePassDesc const& desc);
    ComputePass(ComputePass&& rhs);
    ComputePass& operator=(ComputePass&& rhs);

    ~ComputePass();

    virtual void Begin(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Apply(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever) override;

    void RegisterMaterialData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey);
    void RegisterResourceUsage(ResourceKey const& key, ComputePassResourceUsage usage);


private:
    struct MaterialDelegatedData
    {
        MaterialDelegatedData(MaterialKey const& materialKey, std::uint32_t materialPerPassDataId, PipelineKey const& pipelineKey)
            : materialKey_{ materialKey }, materialPerPassDataId_{ materialPerPassDataId }, pipelineKey_{ pipelineKey } { }

        MaterialKey materialKey_;
        std::uint32_t materialPerPassDataId_;

        PipelineKey pipelineKey_;
    };

    struct ResourceUsageData
    {
        ResourceUsageData(ResourceKey const& key, ComputePassResourceUsage usage)
            : resourceKey_{ key }, usage_{ usage } { }

        ResourceKey resourceKey_;
        ComputePassResourceUsage usage_;
    };

private:
    Root* root_;

    VAL::ImportTable* table_;
    VAL::Device* device_;

    VAL::ResourceRendererProxy* resourceProxy_;
    VAL::PipelineFactory* pipelineFactory_;
    VAL::DescriptorLayoutController* descriptorLayoutController_;

    std::vector<MaterialDelegatedData> materialDelegatedData_;
    std::vector<ResourceUsageData> resourceUsageInfos_;
};


}