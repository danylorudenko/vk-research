#pragma once

#include "../class_features/NonCopyable.hpp"
#include "runtime/FramedDescriptorsHub.hpp"
#include "pipeline/DescriptorLayout.hpp"
#include "resources/ResourcesController.hpp"
#include "buffer/BufferView.hpp"
#include "image/ImageView.hpp"
#include "ProxyHandles.hpp"

namespace VKW
{

class ImportTable;
class Device;
class BuffersProvider;
class ImagesProvider;
class DescriptorSetController;
class DescriptorLayoutController;
class FramebufferController;

struct BufferViewDesc;
struct ImageViewDesc;


///////////////////////////////////
// descriptions
struct ProxyDescriptorDesc
{
    union {
        struct {
            ImageViewHandle imageViewHandle_;
            VkSampler sampler_;
            ImageUsage usage_;
        } imageDesc_;

        struct {
            BufferViewHandle bufferViewHandle_;
        } bufferView_;

        struct {
            BufferViewHandle pureBufferViewHandle_;
            std::uint32_t offset_;
            std::uint32_t size_;
        } bufferInfo_;
    } frames_[FramedDescriptorsHub::MAX_FRAMES_COUNT];
};

struct ProxyFramebufferDesc
{
    VKW::RenderPassHandle renderPass_;

    std::uint32_t width_;
    std::uint32_t height_;

    struct {
        ImageViewHandle colorAttachments_[RenderPass::MAX_COLOR_ATTACHMENTS];
        ImageViewHandle* depthStencilAttachment;
    } frames_[FramedDescriptorsHub::MAX_FRAMES_COUNT];
};
///////////////////////////////////


struct ResourceRendererProxyDesc
{
    ImportTable* table_;
    Device* device_;
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
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


    ProxySetHandle CreateSet(DescriptorSetLayoutHandle layout);
    void WriteSet(ProxySetHandle handle, ProxyDescriptorDesc* descriptions);

    ProxyBufferHandle CreateBuffer(BufferViewDesc const& desc);
    ProxyImageHandle CreateImage(ImageViewDesc const& desc);

    ProxyFramebufferHandle CreateFramebuffer(ProxyFramebufferDesc const& desc);


private:
    struct DescriptorWriteData;

    static void DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& pDescriptorWriteData, VkImageView view);
    static void DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler);
    static void DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view);
    static void DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size);

private:

    ImportTable* table_;
    Device* device_;
    BuffersProvider * buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
    FramebufferController* framebufferController_;
    FramedDescriptorsHub* framedDescriptorsHub_;

};

}