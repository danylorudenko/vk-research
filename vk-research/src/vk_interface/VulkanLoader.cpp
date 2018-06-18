#include "VulkanLoader.hpp"

#include <vector>
#include <algorithm>

namespace VKW
{

VulkanLoader::VulkanLoader()
    : vulkanLibrary_{ "vulkan-1.dll" }
    , vulkanTable_{ vulkanLibrary_ }
{
    std::vector<std::string> const requiredLayers{ "VK_LAYER_LUNARG_standard_validation" };
    std::vector<std::string> const requiredExtensions{ VK_KHR_WIN32_SURFACE_EXTENSION_NAME, "VK_KHR_swapchain" };

    std::uint32_t layerPropertiesCount = 0;
    std::vector<VkLayerProperties> instanceLayerProperties;

    vulkanTable_.vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
    instanceLayerProperties.resize(layerPropertiesCount);
    vulkanTable_.vkEnumerateInstanceLayerProperties(&layerPropertiesCount, instanceLayerProperties.data());

    for (auto const& requiredLayer : requiredLayers) {
        auto const result = std::find_if(instanceLayerProperties.begin(), instanceLayerProperties.end(), [&requiredLayer](auto const& layer)
        {
            return requiredLayer == layer.layerName;
        });

        assert(result != instanceLayerProperties.end() && "Implementation does not support layer required by the application.");
    }

    std::uint32_t extensionPropertiesCount;
    std::vector<VkExtensionProperties> extensionProperties;
    
    vulkanTable_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr);
    extensionProperties.resize(extensionPropertiesCount);
    vulkanTable_.vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties.data());

    for (auto const& requiredExtension : requiredExtensions) {
        auto const result = std::find_if(extensionProperties.begin(), extensionProperties.end(), [&requiredExtension](auto const& extensionProp)
        {
            return requiredExtension == extensionProp.extensionName;
        });

        assert(result != extensionProperties.end() && "Implementation does not support extension required by the application.");
    }

    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "VulkanResearch";
    applicationInfo.pEngineName = "VulkanEngine";
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.applicationVersion = 0;
    applicationInfo.engineVersion = 0;

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    
}

VulkanLoader::~VulkanLoader() = default;

VulkanImportTable const& VulkanLoader::Table() const
{
    return vulkanTable_;
}

}