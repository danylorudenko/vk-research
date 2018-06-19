#include "VulkanLoader.hpp"

#include <vector>
#include <algorithm>

namespace VKW
{

VulkanLoader::VulkanLoader()
    : vulkanLibrary_{ "vulkan-1.dll" }
    , table_{ vulkanLibrary_ }
{
    std::vector<std::string> const requiredInstanceLayers{ "VK_LAYER_LUNARG_standard_validation" };
    std::vector<std::string> const requiredInstanceExtensions{ VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    std::uint32_t layerPropertiesCount = 0;
    std::vector<VkLayerProperties> instanceLayerProperties;

    std::uint32_t extensionPropertiesCount;
    std::vector<VkExtensionProperties> extensionProperties;


    {
        table_.vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
        instanceLayerProperties.resize(layerPropertiesCount);
        table_.vkEnumerateInstanceLayerProperties(&layerPropertiesCount, instanceLayerProperties.data());

        for (auto const& requiredLayer : requiredInstanceLayers) {
            auto const result = std::find_if(instanceLayerProperties.begin(), instanceLayerProperties.end(), [&requiredLayer](auto const& layer)
            {
                return requiredLayer == layer.layerName;
            });

            assert(result != instanceLayerProperties.end() && "Implementation does not support layer required by the application.");
        }
    }
    

    {
        table_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr);
        extensionProperties.resize(extensionPropertiesCount);
        table_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties.data());

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
    instanceCreateInfo.enabledLayerCount = enabledLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    
    table_.vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);


    //"VK_KHR_swapchain" - for device
}

VulkanLoader::~VulkanLoader()
{
    if(instance_)
        table_.vkDestroyInstance(instance_, nullptr);

    instance_ = nullptr;
}

VulkanImportTable const& VulkanLoader::Table() const
{
    return table_;
}

}