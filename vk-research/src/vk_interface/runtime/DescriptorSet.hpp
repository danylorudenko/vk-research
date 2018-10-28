#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "../pipeline/DescriptorLayout.hpp"

#include <vulkan/vulkan.h>

namespace VKW
{

class DescriptorSet
{
public:
    DescriptorSet();
    DescriptorSet(VkDescriptorSet setHandle);

    DescriptorSet(DescriptorSet&& rhs);
    DescriptorSet& operator=(DescriptorSet&& rhs);

    VkDescriptorSet Handle() const;

private:
    VkDescriptorSet handle_;
    DescriptorSetLayoutHandle layout_;
};

}