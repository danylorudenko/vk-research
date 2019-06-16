#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "Material.hpp"
#include "RootDef.hpp"

namespace VKW
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
    virtual void Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) = 0;
    virtual void Render(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) = 0;
    virtual void End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) = 0;
};


struct GraphicsPassDesc
{
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* proxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;
    VKW::FramedDescriptorsHub* framedDescriptorsHub_;
    VKW::ImagesProvider* imagesProvider_;

    std::uint32_t width_;
    std::uint32_t height_;

    std::uint32_t colorAttachmentCount_;
    struct ColorAttachment
    {
        VKW::ProxyImageHandle handle_;
        VKW::RenderPassAttachmentUsage usage_;
    }
    colorAttachments_[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    VKW::ProxyImageHandle* depthStencilAttachment_;
    
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

    VKW::RenderPassHandle VKWRenderPass() const;

    virtual void Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Render(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;

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

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;

    VKW::RenderPassHandle vkRenderPass_;
    VKW::ProxyFramebufferHandle framebuffer_;
    std::uint32_t width_;
    std::uint32_t height_;

    std::vector<MaterialDelegatedData> materialDelegatedData_; 
};


struct ComputePassDesc
{
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* proxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;
    VKW::FramedDescriptorsHub* framedDescriptorsHub_;
    VKW::ImagesProvider* imagesProvider_;
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

    virtual void Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void Render(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;
    virtual void End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever) override;

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

private:
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;

    std::vector<MaterialDelegatedData> materialDelegatedData_;
};


}