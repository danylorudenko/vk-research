#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "VulkanImportTable.hpp"

namespace VKW
{

class VulkanLoader final
    : public NonCopyable
{
public:
    VulkanLoader();

    VulkanLoader(VulkanLoader&& rhs) = default;
    VulkanLoader& operator=(VulkanLoader&& rhs) = default;

    ~VulkanLoader();

    VulkanImportTable const& Table() const;

private:
    DynamicLibrary vulkanLibrary_;
    VulkanImportTable vulkanTable_;

};

}