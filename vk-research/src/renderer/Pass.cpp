#include "Pass.hpp"
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"

#include <utility>

namespace Render
{

Pass::Pass()
    : resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
{

}

Pass::Pass(PassDesc const& desc)
    : resourceProxy_{ desc.proxy_ }
    , renderPassController_{ desc.renderPassController_ }
{
    vkRenderPass_ = renderPassController_->AssembleRenderPass(desc.renderPassDesc_);

    VKW::ProxyFramebufferDesc framebufferProxyDesc;
    framebufferProxyDesc.renderPass_ = vkRenderPass_;
    framebufferProxyDesc.width_ = desc.width_;
    framebufferProxyDesc.height_ = desc.height_;

    for (auto j = 0u; j < desc.colorAttachmentCount_; ++j) {
        framebufferProxyDesc.colorAttachments_[j] = desc.colorAttachments_[j];
    }

    framebufferProxyDesc.depthStencilAttachment_ = desc.depthStencilAttachment_;
    
    framebuffer_ = resourceProxy_->CreateFramebuffer(framebufferProxyDesc);
}

Pass::Pass(Pass&& rhs)
    : resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
{
    operator=(std::move(rhs));
}

Pass& Pass::operator=(Pass&& rhs)
{
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(vkRenderPass_, rhs.vkRenderPass_);
    std::swap(framebuffer_, rhs.framebuffer_);

    return *this;
}

Pass::~Pass()
{
    
}

}