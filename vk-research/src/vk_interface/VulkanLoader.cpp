#include "VulkanLoader.hpp"
#include <iostream>


namespace VKW
{

VulkanLoader::VulkanLoader(bool debug)
    : vulkanLibrary_{ std::make_unique<DynamicLibrary>("vulkan-1.dll") }
    , table_{ std::make_unique<VulkanImportTable>(*vulkanLibrary_) }
{

    auto instanceExtensions = std::vector<std::string>{ "VK_KHR_surface", VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    if (debug)
        instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    auto instanceLayers = std::vector<std::string>{};
    if (debug)
        instanceLayers.emplace_back("VK_LAYER_LUNARG_standard_validation");


    instance_ = std::make_unique<VKW::Instance>( 
        table_.get(), 
        instanceExtensions,
        instanceLayers,
        debug
    );

    device_ = std::make_unique<VKW::Device>( 
        table_.get(), 
        *instance_, 
        std::vector<std::string>{ "VK_KHR_swapchain" } 
    );
}

VulkanLoader::~VulkanLoader()
{
    
}

VulkanImportTable const& VulkanLoader::Table() const
{
    return *table_;
}

Device& VulkanLoader::Device()
{
    return *device_;
}

}