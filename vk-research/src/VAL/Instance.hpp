#pragma once

#include <vector>
#include <string>

#include "..\class_features\NonCopyable.hpp"

#include "ImportTable.hpp"

namespace VAL
{

struct InstanceDesc
{
    ImportTable* table_;
    std::vector<std::string> requiredInstanceExtensions_;
    std::vector<std::string> requiredInstanceLayers_;
    bool debug_;
};

class Instance
    : public NonCopyable
{
public:
    Instance();
    Instance(InstanceDesc const& decs);

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
    VKW::ImportTable* table_;
    VkInstance instance_;
    VkDebugReportCallbackEXT debugCallback_;
};

}