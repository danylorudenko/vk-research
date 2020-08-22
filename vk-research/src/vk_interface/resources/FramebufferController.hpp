#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\resources\Framebuffer.hpp>
#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\pipeline\RenderPass.hpp>

#include <vector>


namespace VKW
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