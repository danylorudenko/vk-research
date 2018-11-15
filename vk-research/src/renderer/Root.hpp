#pragma once

#include "..\class_features\NonCopyable.hpp"

#include <unordered_map>
#include <map>

#include "RootDef.hpp"
#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\buffer\BuffersProvider.hpp"
#include "..\vk_interface\image\ImagesProvider.hpp"

#include "Pass.hpp"

namespace VKW
{
class ResourceRendererProxy;
}

namespace Render
{

struct RootPassDesc
{
    VKW::RenderPassDesc vkRenderPassDesc;

    std::uint32_t colorAttachmentsCount;
    ResourceKey colorAttachments[VKW::RenderPass::MAX_COLOR_ATTACHMENTS];
    ResourceKey depthStencilAttachment;
};

struct RootDesc
{
    VKW::ResourceRendererProxy* resourceProxy_;
    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;
};

class Root
    : public NonCopyable
{
public:
    using GlobalImagesMap = std::unordered_map<ResourceKey, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VKW::ProxyBufferHandle>;

    using RenderPassMap = std::map<RenderPassKey, Pass>;

    Root();
    Root(RootDesc const& desc);
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();


    void DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc);
    VKW::ProxyBufferHandle FindGlobalBuffer(ResourceKey const& key);

    void DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc);
    VKW::ProxyImageHandle FindGlobalImage(ResourceKey const& key);

    void DefineRenderPass(RenderPassKey const& key, RootPassDesc const& desc);

private:
    VKW::ResourceRendererProxy* resourceProxy_;

    std::uint32_t defaultFramebufferWidth_;
    std::uint32_t defaultFramebufferHeight_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;

    RenderPassMap renderPassMap_;

};

}