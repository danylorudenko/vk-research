#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "RootDef.hpp"

namespace VKW
{
class ResourceRendererProxy;
class RenderPassController;
struct FramedDescriptorsHub;
struct RenderPassDesc;
}

namespace Render
{

class Root;

struct PassDesc
{
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

private:
    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;

    VKW::RenderPassHandle vkRenderPass_;
    VKW::ProxyFramebufferHandle framebuffer_;
};

}