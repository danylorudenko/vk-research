#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "RootDef.hpp"

namespace VKW
{
class ResourceRendererProxy;
class RenderPassController;
struct RenderPassDesc;
}

namespace Render
{

struct PassDesc
{
    VKW::ResourceRendererProxy* proxy_;
    VKW::RenderPassController* renderPassController_;

    VKW::RenderPassDesc renderPassDesc_;

    std::uint32_t colorAttachmentCount_;
    ResourceKey colorAttachments[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    ResourceKey depthStencilAttachment;

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

private:
    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::RenderPassController* renderPassController_;

    VKW::RenderPassHandle vkRenderPass_;
    VKW::ProxyFramebufferHandle framebuffer_;
};

}