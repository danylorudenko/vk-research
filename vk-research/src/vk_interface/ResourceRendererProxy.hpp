#pragma once

#include "../class_features/NonCopyable.hpp"
#include "pipeline/DescriptorLayoutController.hpp"
#include "ProxyHandles.hpp"

namespace VKW
{

class BuffersProvider;
class ImagesProvider;
class DescriptorSetsController; struct DescriptorDesc;
class FramedDescriptorsHub;


///////////////////////////////////
// descriptions

///////////////////////////////////


struct ResourceRendererProxyDesc
{
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
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
    void WriteSet(ProxySetHandle handle, DescriptorDesc* descriptions);

private:
    struct DescriptorWriteData;

    static void DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& pDescriptorWriteData, VkImageView view);
    static void DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler);
    static void DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view);
    static void DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size);

private:
    BuffersProvider * buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
    FramedDescriptorsHub* framedDescriptorsHub_;

};

}