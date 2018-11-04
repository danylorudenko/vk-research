#pragma once

#include "../class_features/NonCopyable.hpp"


namespace VKW
{

class BuffersProvider;
class ImagesProvider;
class DescriptorSetsController;
class FramedDescriptorsHub;

struct ResourceRendererProxyDesc
{
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
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




private:
    BuffersProvider * buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorSetsController* descriptorSetsController_;
    FramedDescriptorsHub* framedDescriptorsHub_;
};

}