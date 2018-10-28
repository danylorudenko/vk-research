#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "DescriptorSet.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;


struct DescriptorSetDesc
{
    DescriptorSetLayoutHandle layout_;
};


struct DescriptorSetControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class DescriptorSetController
    : public NonCopyable
{
    DescriptorSetController();
    DescriptorSetController(DescriptorSetControllerDesc const& desc);

    DescriptorSetController(DescriptorSetController&& rhs);
    DescriptorSetController& operator=(DescriptorSetController&& rhs);

    ~DescriptorSetController();

private:
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSet*> descriptorSets_;
};

}