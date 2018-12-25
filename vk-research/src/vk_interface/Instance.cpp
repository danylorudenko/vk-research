#include <utility>
#include <algorithm>
#include <string>

#include "Instance.hpp"
#include "Tools.hpp"

namespace VKW
{

Instance::Instance()
    : instance_{ VK_NULL_HANDLE }
    , table_{ nullptr }
    , debugCallback_{ VK_NULL_HANDLE }
{
    
}

Instance::Instance(InstanceDesc const& desc)
    : instance_{ VK_NULL_HANDLE }
    , table_{ desc.table_ }
    , debugCallback_{ VK_NULL_HANDLE }
{
    std::uint32_t layerPropertiesCount = 0;
    std::vector<VkLayerProperties> instanceLayerProperties;

    std::uint32_t extensionPropertiesCount;
    std::vector<VkExtensionProperties> extensionProperties;


    {
        VK_ASSERT(table_->vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr));
        instanceLayerProperties.resize(layerPropertiesCount);
        VK_ASSERT(table_->vkEnumerateInstanceLayerProperties(&layerPropertiesCount, instanceLayerProperties.data()));

        for (auto const& requiredLayer : desc.requiredInstanceLayers_) {
            auto const result = std::find_if(instanceLayerProperties.begin(), instanceLayerProperties.end(), [&requiredLayer](auto const& layer)
            {
                return requiredLayer == layer.layerName;
            });

            assert(result != instanceLayerProperties.end() && "Implementation does not support layer required by the application.");
        }
    }


    {
        VK_ASSERT(table_->vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr));
        extensionProperties.resize(extensionPropertiesCount);
        VK_ASSERT(table_->vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties.data()));

        for (auto const& requiredExtension : desc.requiredInstanceExtensions_) {
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

    std::transform(
        desc.requiredInstanceLayers_.begin(), desc.requiredInstanceLayers_.end(), 
        std::back_inserter(enabledLayers), [](auto const& layer) {
        return layer.c_str();
    });

    std::transform(desc.requiredInstanceExtensions_.begin(), desc.requiredInstanceExtensions_.end(), 
        std::back_inserter(enabledExtensions), [](auto const& extension) {
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

    VK_ASSERT(table_->vkCreateInstance(&instanceCreateInfo, nullptr, &instance_));

    table_->GetInstanceProcAddresses(instance_);

    // Debug callbacks setup
    if (desc.debug_) {
        VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
        debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugCallbackCreateInfo.pNext = nullptr;
        debugCallbackCreateInfo.pfnCallback = &Instance::DebugCallback;
        debugCallbackCreateInfo.flags = 
            /*VK_DEBUG_REPORT_INFORMATION_BIT_EXT |*/
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_ERROR_BIT_EXT/* |
            VK_DEBUG_REPORT_DEBUG_BIT_EXT*/;
        debugCallbackCreateInfo.pUserData = nullptr;

        VK_ASSERT(table_->vkCreateDebugReportCallbackEXT(instance_, &debugCallbackCreateInfo, nullptr, &debugCallback_));
    }
}

Instance::Instance(Instance&& rhs)
    : instance_{ VK_NULL_HANDLE }
    , table_{ nullptr }
    , debugCallback_{ VK_NULL_HANDLE }
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
    if (debugCallback_)
        table_->vkDestroyDebugReportCallbackEXT(instance_, debugCallback_, nullptr);

    if (instance_)
        table_->vkDestroyInstance(instance_, nullptr);

    instance_ = VK_NULL_HANDLE;
}

VkBool32 Instance::DebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT type,
    std::uint64_t object,
    std::size_t location,
    std::int32_t code,
    char const* layerPrefix,
    char const* msg,
    void* userData)
{
    std::string flagsString;
    
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        flagsString += "INFORMATION|";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        flagsString += "WARNING|";
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        flagsString += "PERFORMANCE_WARNING|";
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        flagsString += "ERROR|";
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        flagsString += "DEBUG|";
    

    char const* objTypeStr = nullptr;
    switch (type) {
    case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT";
        break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
        objTypeStr = "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT";
        break;
    default:
        objTypeStr = "UNKNOWN";
        break;
    }

    std::cout 
        << std::endl << "DEBUG_LAYER"
        << std::endl << "Message type: " << flagsString
        << std::endl << "Layer prefix: " << layerPrefix
        << std::endl << "Message: " << msg
        << std::endl << "Object type: " << objTypeStr << ", id: (" << object << ")" << std::endl << std::endl;

    return VK_FALSE;
}

}