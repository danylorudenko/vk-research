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
    Instance(VulkanImportTable* table, std::vector<std::string> const& requiredInstanceExtensions, std::vector<std::string> const& requiredInstanceLayers, bool debug);

    Instance(Instance&& rhs);
    Instance& operator=(Instance&& rhs);
    
    VkInstance Handle() const;

    operator bool() const;

    ~Instance();

    static VKAPI_ATTR VkBool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT type,
        std::uint64_t object,
        std::size_t location,
        std::int32_t code,
        char const* layerPrefix,
        char const* msg,
        void* userData
    );


private:
    VKW::VulkanImportTable* table_;
    VkInstance instance_;
    VkDebugReportCallbackEXT debugCallback_;
};

}