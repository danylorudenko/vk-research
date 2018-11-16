#include <utility>
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"

namespace Render
{

Root::Root()
    : resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , defaultFramebufferWidth_{ 0 }
    , defaultFramebufferHeight_{ 0 }
{

}

Root::Root(RootDesc const& desc)
    : resourceProxy_{ desc.resourceProxy_ }
    , renderPassController_{ desc.renderPassController_ }
    , defaultFramebufferWidth_{ desc.defaultFramebufferWidth_ }
    , defaultFramebufferHeight_{ desc.defaultFramebufferHeight_ }
{

}

Root::Root(Root&& rhs)
    : resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , defaultFramebufferWidth_{ 0 }
    , defaultFramebufferHeight_{ 0 }
{
    operator=(std::move(rhs));
}

Root& Root::operator=(Root&& rhs)
{
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(defaultFramebufferWidth_, rhs.defaultFramebufferWidth_);
    std::swap(defaultFramebufferHeight_, rhs.defaultFramebufferHeight_);
    std::swap(globalBuffers_, rhs.globalBuffers_);
    std::swap(globalImages_, rhs.globalImages_);
    std::swap(renderPassMap_, rhs.renderPassMap_);

    return *this;
}

Root::~Root()
{

}

void Root::DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc)
{
    VKW::ProxyBufferHandle bufferHandle = resourceProxy_->CreateBuffer(desc);
    globalBuffers_[key] = bufferHandle;
}

VKW::ProxyBufferHandle Root::FindGlobalBuffer(ResourceKey const& key)
{
    return globalBuffers_[key];
}

void Root::DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc)
{
    VKW::ProxyImageHandle imageHandle = resourceProxy_->CreateImage(desc);
    globalImages_[key] = imageHandle;
}

VKW::ProxyImageHandle Root::FindGlobalImage(ResourceKey const& key)
{
    return globalImages_[key];
}

void Root::DefineRenderPass(RenderPassKey const& key, RootPassDesc const& desc)
{
    PassDesc passDesc;
    passDesc.proxy_ = resourceProxy_;
    passDesc.renderPassController_ = renderPassController_;
    passDesc.renderPassDesc_ = desc.vkRenderPassDesc_;
    passDesc.width_ = defaultFramebufferWidth_;
    passDesc.height_ = defaultFramebufferHeight_;
    passDesc.colorAttachmentCount_ = desc.colorAttachmentsCount_;
    for (auto i = 0u; i < passDesc.colorAttachmentCount_; ++i) {
        passDesc.colorAttachments_[i] = globalImages_[desc.colorAttachments_[i]];
    }
    passDesc.depthStencilAttachment_ = desc.depthStencilAttachment_.size() > 0 ? &globalImages_[desc.depthStencilAttachment_] : nullptr;

    renderPassMap_[key] = Pass{ passDesc };
}

}