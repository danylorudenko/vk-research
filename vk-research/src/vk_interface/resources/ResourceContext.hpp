#pragma once

#include <vector>
#include "../../class_features/NonCopyable.hpp"
#include "Resource.hpp"
#include "Framebuffer.hpp"

namespace VKW
{

class ResourceContext
{
public:
    ResourceContext();
    ResourceContext(ResourceContext&& rhs);
    ResourceContext& operator=(ResourceContext&& rhs);

    std::vector<FramebufferHandle> framebuffers_;
    
    // These should be descriptor sets
    //std::vector<BufferResourceHandle> bufferResources_;
    //std::vector<ImageResourceHandle> imageResources_;

};

}