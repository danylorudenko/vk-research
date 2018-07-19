#pragma once

#include "..\..\class_features\NonCopyable.hpp"




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