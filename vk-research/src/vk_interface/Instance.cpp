#include <utility>
#include <algorithm>

#include "Instance.hpp"
#include "Tools.hpp"

namespace VKW
{

Instance::Instance()
    : instance_{ VK_NULL_HANDLE }
    , table_{ nullptr }
{
    
}

Instance::Instance(VulkanImportTable* importTable, std::vector<std::string> const& requiredInstanceExtensions, std::vector<std::string> const& requiredInstanceLayers)
    : instance_{ VK_NULL_HANDLE }
    , table_{ importTable }
{
    std::uint32_t layerPropertiesCount = 0;
    std::vector<VkLayerProperties> instanceLayerProperties;

    std::uint32_t extensionPropertiesCount;
    std::vector<VkExtensionProperties> extensionProperties;


    {
        VK_ASSERT(importTable->vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr));
        instanceLayerProperties.resize(layerPropertiesCount);
        VK_ASSERT(importTable->vkEnumerateInstanceLayerProperties(&layerPropertiesCount, instanceLayerProperties.data()));

        for (auto const& requiredLayer : requiredInstanceLayers) {
            auto const result = std::find_if(instanceLayerProperties.begin(), instanceLayerProperties.end(), [&requiredLayer](auto const& layer)
            {
                return requiredLayer == layer.layerName;
            });

            assert(result != instanceLayerProperties.end() && "Implementation does not support layer required by the application.");
        }
    }


    {
        VK_ASSERT(importTable->vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr));
        extensionProperties.resize(extensionPropertiesCount);
        VK_ASSERT(importTable->vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties.data()));

        for (auto const& requiredExtension : requiredInstanceExtensions) {
            auto const result = std::find_if(extensionProperties.begin(), extensionProperties.end(), [&requiredExtension](auto const& extensionProp)
            {
                return requiredExtension == extensionProp.extensionName;
            });

            assert(result != extensionProperties.end() && "Implementation does not support extension required by the application.");
        }
    }


    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "VulkanResearch";
    applicationInfo.pEngineName = "VulkanEngine";
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.applicationVersion = 0;
    applicationInfo.engineVersion = 0;


    std::vector<char const*> enabledLayers;
    std::vector<char const*> enabledExtensions;

    enabledLayers.resize(requiredInstanceLayers.size());
    std::transform(requiredInstanceLayers.begin(), requiredInstanceLayers.end(), enabledLayers.begin(), [](auto const& layer) {
        return layer.c_str();
    });

    enabledExtensions.resize(requiredInstanceExtensions.size());
    std::transform(requiredInstanceExtensions.begin(), requiredInstanceExtensions.end(), enabledExtensions.begin(), [](auto const& extension) {
        return extension.c_str();
    });

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(enabledLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VK_ASSERT(importTable->vkCreateInstance(&instanceCreateInfo, nullptr, &instance_));

    importTable->GetInstanceProcAddresses(instance_);
}

Instance::Instance(Instance&& rhs)
{
    operator=(std::move(rhs));
}

Instance& Instance::operator=(Instance&& rhs)
{
    std::swap(rhs.instance_, instance_);
    std::swap(rhs.table_, table_);

    return *this;
}

Instance::operator bool() const
{
    return instance_ != VK_NULL_HANDLE;
}

VkInstance Instance::Handle() const
{
    return instance_;
}

Instance::~Instance()
{
    if (*this)
        table_->vkDestroyInstance(instance_, nullptr);

    instance_ = VK_NULL_HANDLE;
}

}