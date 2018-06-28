#pragma once

#include <vector>
#include <string>

#include "..\class_features\NonCopyable.hpp"

#include "VulkanImportTable.hpp"

namespace VKW
{

class Instance
    : public NonCopyable
{
public:
    Instance();
    Instance(VulkanImportTable& table, std::vector<std::string> const& requiredInstanceExtensions, std::vector<std::string> const& requiredInstanceLayers);

    Instance(Instance&& rhs);
    Instance& operator=(Instance&& rhs);

    operator bool() const;

    ~Instance();


private:
    VKW::VulkanImportTable* table_;
    VkInstance instance_;
};

}