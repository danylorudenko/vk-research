#include "VulkanLoader.hpp"
#include <iostream>


namespace VKW
{

VulkanLoader::VulkanLoader()
    : vulkanLibrary_{ "vulkan-1.dll" }
    , table_{ vulkanLibrary_ }
{

    instance_ = VKW::Instance{ 
        &table_, 
        { VK_KHR_WIN32_SURFACE_EXTENSION_NAME }, 
        { "VK_LAYER_LUNARG_standard_validation" } 
    };

    device_ = VKW::Device{ 
        &table_, 
        instance_, 
        { "VK_KHR_swapchain" } 
    };
}

VulkanLoader::~VulkanLoader()
{
    
}

VulkanImportTable const& VulkanLoader::Table() const
{
    return table_;
}

}