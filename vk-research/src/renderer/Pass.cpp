#include "Pass.hpp"
#include "../vk_interface/ResourceRendererProxy.hpp"
#include "../vk_interface/pipeline/RenderPassController.hpp"

namespace Render
{

Pass::Pass()
    : resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , vkRenderPass_{}
{

}

Pass::Pass(PassDesc const& desc)
    : resourceProxy_{ desc.proxy_ }
    , renderPassController_{ desc.renderPassController_ }
    , vkRenderPass_{}
{
    vkRenderPass_ = renderPassController_->AssembleRenderPass(*desc.renderPassDesc_);

    // now own framebuffer needed form proxy
}

}