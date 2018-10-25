#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "DescriptorLayout.hpp"

#include <vector>

namespace VKW
{

class Device;
class ImportTable;

struct DescriptorLayoutControllerDesc
{
    ImportTable* table_;
    Device* device_;
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
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSetLayout> setLayouts_;
    std::vector<DescriptorPipelineLayout> pipelineLayouts_;
};

}