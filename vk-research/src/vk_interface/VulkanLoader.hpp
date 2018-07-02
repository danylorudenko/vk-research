#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"


#include "VulkanImportTable.hpp"
#include "Instance.hpp"
#include "Device.hpp"

namespace VKW
{

class VulkanLoader final
    : public NonCopyable
{
public:
    VulkanLoader(bool debug = false);

    VulkanLoader(VulkanLoader&& rhs) = default;
    VulkanLoader& operator=(VulkanLoader&& rhs) = default;

    ~VulkanLoader();

    VulkanImportTable const& Table() const;

private:
    DynamicLibrary vulkanLibrary_;
    VulkanImportTable table_;

    Instance instance_;
    Device device_;

};

}