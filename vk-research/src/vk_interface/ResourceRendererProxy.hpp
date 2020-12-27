#pragma once

#include <class_features\NonCopyable.hpp>

#include <vk_interface\pipeline\DescriptorLayout.hpp>
#include <vk_interface\resources\ResourcesController.hpp>
#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\ProxyHandles.hpp>

namespace VKW
{

class ImportTable;
class Device;
class BuffersProvider;
class ImagesProvider;
class RenderPassController;
class DescriptorSetController;
class DescriptorLayoutController;
class FramebufferController;

struct BufferViewDesc;
struct ImageViewDesc;
struct FramedDescriptorsHub;



//////////////////////////////////////
// descriptions
struct ProxyDescriptorWriteDesc
{
    union Union 
    {
        struct ImageDesc 
        {
            ImageViewHandle imageViewHandle_;
            VkSampler sampler_;
            VkImageLayout layout_;
        } 
        imageDesc_;

        struct BufferViewDesc 
        {
            BufferViewHandle bufferViewHandle_;
        } bufferView_;

        struct PureBufferDesc {
            BufferViewHandle pureBufferViewHandle_;
            std::uint32_t offset_;
            std::uint32_t size_;
        } pureBufferDesc_;
    } 
    frames_[CONSTANTS::MAX_FRAMES_BUFFERING];
};

struct ProxyFramebufferDesc
{
    VKW::RenderPassHandle renderPass_;

    std::uint32_t width_;
    std::uint32_t height_;

    ProxyImageHandle colorAttachments_[RenderPass::MAX_COLOR_ATTACHMENTS];
    ProxyImageHandle* depthStencilAttachment_;
};
//////////////////////////////////////



struct ResourceRendererProxyDesc
{
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
    ResourcesController* resourcesController_;
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
    RenderPassController* renderPassController_;
    FramebufferController* framebufferController_;
    FramedDescriptorsHub* framedDescriptorsHub_;
};

class ResourceRendererProxy
    : public NonCopyable
{
public:
    ResourceRendererProxy();
    ResourceRendererProxy(ResourceRendererProxyDesc const& desc);
    ResourceRendererProxy(ResourceRendererProxy&& rhs);
    ResourceRendererProxy& operator=(ResourceRendererProxy&& rhs);
    ~ResourceRendererProxy();

    std::uint32_t FramesCount() const;

    ProxyImageHandle RegisterSwapchainImageViews();

    ProxySetHandle CreateSet(DescriptorSetLayoutHandle layout);
    void WriteSet(ProxySetHandle handle, ProxyDescriptorWriteDesc* descriptions);
    DescriptorSetHandle GetDescriptorSetHandle(ProxySetHandle handle, std::uint32_t context);
    DescriptorSet* GetDescriptorSet(ProxySetHandle handle, std::uint32_t context);

    ProxyBufferHandle CreateBuffer(BufferViewDesc const& desc);
    BufferView* GetBufferView(ProxyBufferHandle handle, std::uint32_t context);
    BufferViewHandle GetBufferViewHandle(ProxyBufferHandle handle, std::uint32_t context);

    ProxyImageHandle CreateImage(ImageViewDesc const& desc);
    ImageViewHandle GetImageViewHandle(ProxyImageHandle handle, std::uint32_t context);
    ImageView* GetImageView(ProxyImageHandle handle, std::uint32_t context);

    VkSampler GetDefaultSampler();

    ProxyFramebufferHandle CreateFramebuffer(ProxyFramebufferDesc const& desc);
    Framebuffer* GetFramebuffer(ProxyFramebufferHandle handle, std::uint32_t context);

    void* MapBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context);
    void FlushBuffer(VKW::ProxyBufferHandle handle, std::uint32_t context);


private:
    struct DescriptorWriteData;

    static void DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& pDescriptorWriteData, VkImageView view, VkImageLayout layout);
    static void DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler);
    static void DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view);
    static void DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size);

private:

    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
    ResourcesController* resourcesController_;
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
    RenderPassController* renderPassController_;
    FramebufferController* framebufferController_;
    FramedDescriptorsHub* framedDescriptorsHub_;

};

}