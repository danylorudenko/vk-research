#pragma once

#include "../class_features/NonCopyable.hpp"
#include "pipeline/DescriptorLayoutController.hpp"
#include "ProxyHandles.hpp"

namespace VKW
{

class BuffersProvider;
class ImagesProvider;
class DescriptorSetsController;
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


    std::uint32_t CreateSet(DescriptorSetLayoutHandle layout);
    void DestroySet(std::uint32_t handle);


private:
    BuffersProvider * buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
    DescriptorSetController* descriptorSetsController_;
    FramedDescriptorsHub* framedDescriptorsHub_;
};

}