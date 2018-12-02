#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "RootDef.hpp"

namespace VKW
{
class ImportTable;
class Device;
class ResourceRendererProxy;
class RenderPassController;
class Worker;
struct WorkerFrameCommandReciever;
struct FramedDescriptorsHub;
struct RenderPassDesc;
}

namespace Render
{

class Root;

struct PassDesc
{
    Root* root_;

    VKW::ResourceRendererProxy* proxy_;
    VKW::RenderPassController* renderPassController_;
    VKW::FramedDescriptorsHub* framedDescriptorsHub_;
    VKW::ImagesProvider* imagesProvider_;

    std::uint32_t width_;
    std::uint32_t height_;

    std::uint32_t colorAttachmentCount_;
    VKW::ProxyImageHandle colorAttachments_[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    VKW::ProxyImageHandle* depthStencilAttachment_;
    
};

class Pass
    : public NonCopyable
{
public:
    Pass();
    Pass(PassDesc const& desc);
    Pass(Pass&& rhs);
    Pass& operator=(Pass&& rhs);
    ~Pass();

    VKW::RenderPassHandle VKWRenderPass() const;

    void Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever);
    void Render(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever);
    void End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever);

    void AddPipeline(PipelineKey const& pipeline);

private:
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;

    VKW::RenderPassHandle vkRenderPass_;
    VKW::ProxyFramebufferHandle framebuffer_;

    std::vector<PipelineKey> pipelines_; 
};

}