#pragma once

#include "..\class_features\NonCopyable.hpp"

namespace VKW
{

class Instance
{
public:
    Instance();
    //Instance(VulkanImporn)

    Instance(Instance&& rhs);
    Instance& operator=(Instance&& rhs);

    ~Instance();
};

}