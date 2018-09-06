#pragma once

#include "../../class_features/NonCopyable.hpp"

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

    inline operator bool() const;
    inline bool IsNull() const;

private:
    VkDescriptorSet handle_;
};

}