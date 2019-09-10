#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Framebuffer.hpp"
#include "..\image\ImageView.hpp"
#include "..\pipeline\RenderPass.hpp"
#include <vector>
#include <cstdint>

namespace VAL
{

class ImportTable;
class Device;
class ImagesProvider;
class RenderPassController;
class Swapchain;


struct FramebufferDesc
{
    RenderPassHandle renderPass_;

    ImageViewHandle const* colorAttachments_;
    ImageViewHandle const* depthStencilAttachment_;
    std::uint32_t width_;
    std::uint32_t height_;

};

struct FramebufferControllerDesc
{
    ImportTable* table_;
    Device* device_;
    Swapchain* swapchain_;
    ImagesProvider* imagesProvider_;
    RenderPassController* renderPassController_;
};

class FramebufferController
    : public NonCopyable
{
public:
    FramebufferController();
    FramebufferController(FramebufferControllerDesc const& desc);

    FramebufferController(FramebufferController&& rhs);
    FramebufferController& operator=(FramebufferController&& rhs);

    ~FramebufferController();

    FramebufferHandle CreateFramebuffer(FramebufferDesc const& desc);
    void FreeFramebuffer(FramebufferHandle handle);

    Framebuffer* GetFramebuffer(FramebufferHandle handle);

private:
    ImportTable* table_;
    Device* device_;

    Swapchain* swapchain_;
    ImagesProvider* imagesProvider_;
    RenderPassController* renderPassController_;

    std::vector<Framebuffer*> framebuffers_;
};

}