#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "Framebuffer.hpp"
#include "../resources/Resource.hpp"
#include "../pipeline/RenderPass.hpp"
#include <vector>
#include <cstdint>

namespace VKW
{

class ImportTable;
class Device;
class ResourcesController;
class RenderPassController;

struct FramebufferDesc
{
    RenderPassHandle renderPass_;

    std::uint32_t colorAttachmentCount_;
    ImageResourceHandle* colorAttachments;

    ImageResourceHandle depthStencilAttachment;
};

struct FramebufferFactoryDesc
{
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;
    RenderPassController* renderPassController_;
};

class FramebufferController
    : public NonCopyable
{
public:
    FramebufferController();
    FramebufferController(FramebufferFactoryDesc const& desc);

    FramebufferController(FramebufferController&& rhs);
    FramebufferController& operator=(FramebufferController&& rhs);

    ~FramebufferController();

    FramebufferHandle CreateFramebuffer(FramebufferDesc const& desc);
    void FreeFramebuffer(FramebufferHandle handle);

    Framebuffer* GetFramebuffer(FramebufferHandle handle);

private:
    ImportTable* table_;
    Device* device_;

    ResourcesController* resourcesController_;
    RenderPassController* renderPassController_;

    std::vector<Framebuffer> framebuffers_;
};

}