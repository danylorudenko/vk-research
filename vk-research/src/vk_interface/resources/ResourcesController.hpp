#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Resource.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace VKW
{

class ResourcesController
    : public NonCopyable
{
public:
    ResourcesController();
    ResourcesController(ResourcesController&& rhs);
    ResourcesController& operator=(ResourcesController&& rhs);

    ~ResourcesController();

private:
    std::vector<BufferResource> staticBuffers_;
    std::vector<ImageResource> staticImages_;


};


}