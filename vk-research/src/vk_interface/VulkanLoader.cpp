#include "VulkanLoader.hpp"
#include <iostream>


namespace VKW
{

VulkanLoader::VulkanLoader()
    : vulkanLibrary_{ "vulkan-1.dll" }
    , table_{ vulkanLibrary_ }
{
    std::vector<std::string> const requiredInstanceLayers{ "VK_LAYER_LUNARG_standard_validation" };
    std::vector<std::string> const requiredInstanceExtensions{ VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    instance_ = VKW::Instance{ &table_, requiredInstanceExtensions, requiredInstanceLayers };

    //"VK_KHR_swapchain" - for device
}

VulkanLoader::~VulkanLoader()
{
    
}

VulkanImportTable const& VulkanLoader::Table() const
{
    return table_;
}

}