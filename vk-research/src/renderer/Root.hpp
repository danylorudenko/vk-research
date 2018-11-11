#pragma once

#include "..\class_features\NonCopyable.hpp"
#include <unordered_map>
#include <string>

#include "..\vk_interface\ProxyHandles.hpp"
#include "..\vk_interface\buffer\BufferView.hpp"
#include "..\vk_interface\image\ImageView.hpp"

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
    GlobalImagesMap globalImages_;
    GlobalBuffersMap globalBuffers_;

};

}