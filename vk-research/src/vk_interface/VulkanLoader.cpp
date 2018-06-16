#include "VulkanLoader.hpp"

namespace VKW
{

VulkanLoader::VulkanLoader()
    : vulkanLibrary_{ "vulkan-1.dll" }
    , vulkanTable_{ vulkanLibrary_ }
{
}

VulkanLoader::VulkanLoader(VulkanLoader&& rhs) = default;

VulkanLoader& VulkanLoader::operator=(VulkanLoader&& rhs) = default;

VulkanLoader::~VulkanLoader() = default;

VulkanImportTable const& VulkanLoader::Table() const
{
    return vulkanTable_;
}

}