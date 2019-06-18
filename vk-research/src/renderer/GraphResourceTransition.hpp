#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "ResourceUsage.hpp"

namespace Render
{

class GraphResourceTransition
    : public NonCopyable
{
public:
    GraphResourceTransition();
    GraphResourceTransition(GraphResourceTransition&& rhs);
    GraphResourceTransition& operator=(GraphResourceTransition&& rhs);
    ~GraphResourceTransition();


    struct ResourceSlot
    {
        ResourceUsage usageIn_;
        ResourceUsage usageOut_;
    };


    void Apply() {} 
};

}

