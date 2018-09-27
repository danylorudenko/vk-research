#include "FramebufferController.hpp"

#include <utility>
#include "../ImportTable.hpp"
#include "../Device.hpp"
#include "../resources/ResourcesController.hpp"
#include "../pipeline/RenderPassController.hpp"

namespace VKW
{

FramebufferController::FramebufferController()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
    , renderPassController_{ nullptr }
{

}

FramebufferController::FramebufferController(FramebufferFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ desc.resourcesController_ }
    , renderPassController_{ desc.renderPassController_ }
{

}

FramebufferController::FramebufferController(FramebufferController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
    , renderPassController_{ nullptr }
{
    operator=(std::move(rhs));
}

FramebufferController& FramebufferController::operator=(FramebufferController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(renderPassController_, rhs.renderPassController_);

    return *this;
}

FramebufferHandle FramebufferController::CreateFramebuffer(FramebufferDesc const& desc)
{
    RenderPass* renderPass = renderPassController_->GetRenderPass(desc.renderPass_);

    // TODO
}

}