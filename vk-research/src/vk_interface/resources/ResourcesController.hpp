#pragma once

#include "..\..\class_features\NonCopyable.hpp"

#include <vulkan/vulkan.h>

namespace VKW
{

class Image
{
    VkImage handle_;
};

class ImageHandle
{

};



class ResourcesController
    : public NonCopyable
{
public:
    ResourcesController();
    ResourcesController(ResourcesController&& rhs);
    ResourcesController& operator=(ResourcesController&& rhs);

    ~ResourcesController();

private:

};


}