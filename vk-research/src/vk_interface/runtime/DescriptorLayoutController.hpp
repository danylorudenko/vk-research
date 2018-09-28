#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "DescriptorLayout.hpp"

#include <vector>

namespace VKW
{

struct DescriptorLayoutControllerDesc
{

};

class DescriptorLayoutController
    : public NonCopyable
{
    DescriptorLayoutController();
    DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc);

    DescriptorLayoutController(DescriptorLayoutController&& rhs);
    DescriptorLayoutController& operator=(DescriptorLayoutController&& rhs);

    ~DescriptorLayoutController();
    
private:
    std::vector<DescriptorSetLayout> setLayouts_;
    std::vector<DescriptorPipelineLayout> pipelineLayouts_;
};

}