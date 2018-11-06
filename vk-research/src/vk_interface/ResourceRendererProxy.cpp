#include "ResourceRendererProxy.hpp"
#include "buffer/BuffersProvider.hpp"
#include "image/ImagesProvider.hpp"
#include "pipeline/DescriptorLayoutController.hpp"
#include "runtime/DescriptorSetController.hpp"
#include "runtime/FramedDescriptorsHub.hpp"

#include <utility>

namespace VKW
{

ResourceRendererProxy::ResourceRendererProxy()
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
    , descriptorSetsController_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxyDesc const& desc)
    : buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , layoutController_{ desc.layoutController_ }
    , descriptorSetsController_{ desc.descriptorSetsController_ }
    , framedDescriptorsHub_{ desc.framedDescriptorsHub_ }
{

}

ResourceRendererProxy::ResourceRendererProxy(ResourceRendererProxy&& rhs)
    : buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
    , layoutController_{ nullptr }
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

std::uint32_t ResourceRendererProxy::CreateSet(DescriptorSetLayoutHandle layout, ProxyResourceDesc const* membersDesc)
{
    DescriptorSetLayout* layoutPtr = layoutController_->GetDescriptorSetLayout(layout);
    std::uint32_t const layoutMembersCount = layoutPtr->membersCount_;


    for (auto i = 0u; i < layoutMembersCount; ++i) {

        auto& layoutMember = layoutPtr->membersInfo_[i];

    }


    DescriptorSetDesc desc;
    desc.layout_ = layout;

    

    return 0u;
}

}