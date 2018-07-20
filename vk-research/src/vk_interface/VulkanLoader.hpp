#pragma once

#include <memory>

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

    VKW::VulkanImportTable const& Table() const;
    VKW::Device& Device();

private:
    std::unique_ptr<DynamicLibrary> vulkanLibrary_;
    std::unique_ptr<VKW::VulkanImportTable> table_;

    std::unique_ptr<VKW::Instance> instance_;
    std::unique_ptr<VKW::Device> device_;

};

}