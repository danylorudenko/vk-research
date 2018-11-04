#include "ResourceRendererProxy.hpp"
#include "Loader.hpp"

#include <utility>

namespace VKW
{

ResourceRendererProxy::ResourceRendererProxy()
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxyDesc const& desc)
    : buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , descriptorSetsController_{ desc.descriptorSetsController_ }
    , framedDescriptorsHub_{ desc.framedDescriptorsHub_ }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxy&& rhs)
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{
    operator=(std::move(rhs));
}

ResourceRendererProxy& ResourceRendererProxy::operator=(ResourceRendererProxy&& rhs)
{
    std::swap(buffersProvider_, rhs.buffersProvider_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(descriptorSetsController_, rhs.descriptorSetsController_);
    std::swap(framedDescriptorsHub_, rhs.framedDescriptorsHub_);

    return *this;
}

ResourceRendererProxy::~ResourceRendererProxy()
{

}

}