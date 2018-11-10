#pragma once

#include "../vk_interface/pipeline/RenderPass.hpp"

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

    VKW::RenderPassDesc* renderPassDesc_;
};

class Pass
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
};

}