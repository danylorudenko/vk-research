#include "Pass.hpp"
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "..\vk_interface\pipeline\RenderPassController.hpp"

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
    Root& root = *desc.root_;

    vkRenderPass_ = renderPassController_->AssembleRenderPass(desc.renderPassDesc_);
    std::uint32_t const framesCount = desc.framesCount_;

    VKW::ProxyFramebufferDesc framebufferDesc;
    framebufferDesc.renderPass_ = vkRenderPass_;
    framebufferDesc.width_ = desc.width_;
    framebufferDesc.height_ = desc.height_;

    for (auto i = 0u; i < framesCount; ++i) {

        auto& resourceFrame = desc.framedDescriptorsHub_->contexts_[i];
        auto& dstFrame = framebufferDesc.frames_[i]; // TODO That's pretty messed up, who is responsible for what???

        for (auto j = 0u; j < desc.colorAttachmentCount_; ++j) {
            dstFrame.colorAttachments_[j] = desc.colorAttachments_[j];
        }
        
    }
    

    //framebuffer_ = resourceProxy_->CreateFramebuffer()
}

}