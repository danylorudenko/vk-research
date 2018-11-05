#pragma once

#include "../class_features/NonCopyable.hpp"
#include "pipeline/DescriptorLayoutController.hpp"
#include <cstdint>

namespace VKW
{

class BuffersProvider;
class ImagesProvider;
class DescriptorSetsController;
class FramedDescriptorsHub;



///////////////////////////////////
// descriptions
enum ResourceType
{
    RESOURCE_TYPE_UNIFORM_BUFFER,
    RESOURCE_TYPE_SAMPLED_TEXTURE
};

struct ProxyResourceDesc
{
    union {
        struct {
            std::uint32_t size_;
        } uniformBufferDesc_;

        struct {
            std::uint32_t width_;
            std::uint32_t height_;
            VkFormat format_;
        } sampledTextureDesc_;
    };
};
///////////////////////////////////


struct ResourceRendererProxyDesc
{
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetsController* descriptorSetsController_;
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


    std::uint32_t CreateSet(DescriptorSetLayoutHandle layout, ProxyResourceDesc const* membersDesc);
    void DestroySet(std::uint32_t handle);


private:
    BuffersProvider * buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetsController* descriptorSetsController_;
    FramedDescriptorsHub* framedDescriptorsHub_;
};

}