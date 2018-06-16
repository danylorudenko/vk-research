#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "VulkanImportTable.hpp"

namespace VKW
{

class VulkanLoader
    : public NonCopyable
{
public:
    VulkanLoader();

    VulkanLoader(VulkanLoader&& rhs);
    VulkanLoader& operator=(VulkanLoader&& rhs);

    ~VulkanLoader();

    VulkanImportTable const& Table() const;

private:
    DynamicLibrary vulkanLibrary_;
    VulkanImportTable vulkanTable_;

};

}