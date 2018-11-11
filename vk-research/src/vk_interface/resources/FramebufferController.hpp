#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Framebuffer.hpp"
#include "..\image\ImageView.hpp"
#include "..\pipeline\RenderPass.hpp"
#include <vector>
#include <cstdint>

namespace VKW
{

class ImportTable;
class Device;
class ImagesProvider;
class RenderPassController;

struct FramebufferDesc
{
    RenderPassHandle renderPass_;

    ImageViewHandle const* colorAttachments;
    ImageViewHandle const* depthStencilAttachment;
    std::uint32_t width_;
    std::uint32_t height_;

};

struct FramebufferFactoryDesc
{
    ImportTable* table_;
    Device* device_;
    ImagesProvider* imagesProvider_;
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

    ImagesProvider* imagesProvider_;
    RenderPassController* renderPassController_;

    std::vector<Framebuffer*> framebuffers_;
};

}