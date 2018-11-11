#pragma once

#include "..\class_features\NonCopyable.hpp"
#include <unordered_map>
#include <string>

#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\buffer\BuffersProvider.hpp"
#include "..\vk_interface\image\ImagesProvider.hpp"

namespace VKW
{
class ResourceRendererProxy;
}

namespace Render
{

struct RootDesc
{
    VKW::ResourceRendererProxy* resourceProxy_;
};

class Root
    : public NonCopyable
{
public:
    using ResourceKey = std::string;
    using GlobalImagesMap = std::unordered_map<ResourceKey, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceKey, VKW::ProxyBufferHandle>;

    Root();
    Root(RootDesc const& desc);
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();


    void DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc);
    void DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc);

private:
    VKW::ResourceRendererProxy* resourceProxy_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;

};

}