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

class Root
    : public NonCopyable
{
public:
    using ResourceHash = std::string;
    using GlobalImagesMap = std::unordered_map<ResourceHash, VKW::ProxyImageHandle>;
    using GlobalBuffersMap = std::unordered_map<ResourceHash, VKW::ProxyBufferHandle>;

    Root();
    Root(Root&& rhs);
    Root& operator=(Root&& rhs);
    ~Root();


    void DefineGlobalBuffer(ResourceHash const& hash, VKW::BufferViewDesc const& desc);
    void DefineGlobalImage(ResourceHash const& hash, VKW::ImageViewDesc const& desc);

private:
    VKW::ResourceRendererProxy* resourceProxy_;

    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;

};

}