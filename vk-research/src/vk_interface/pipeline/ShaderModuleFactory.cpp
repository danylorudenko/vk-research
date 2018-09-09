#include "ShaderModuleFactory.hpp"
#include <utility>
#include "../ImportTable.hpp"
#include "../Device.hpp"
#include "../Tools.hpp"
#include "../../io/IOManager.hpp"

namespace VKW
{

ShaderModuleFactory::ShaderModuleFactory()
    : table_{ nullptr }
    , device_{ nullptr }
    , ioManager_{ nullptr }
{
}

ShaderModuleFactory::ShaderModuleFactory(ShaderModuleFactoryDesc const& desc)
    : table_{ nullptr }
    , device_{ nullptr }
    , ioManager_{ nullptr }
{
}

ShaderModuleFactory::ShaderModuleFactory(ShaderModuleFactory&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , ioManager_{ nullptr }
{
    operator=(std::move(rhs));
}

ShaderModuleFactory& ShaderModuleFactory::operator=(ShaderModuleFactory&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(ioManager_, rhs.ioManager_);
    std::swap(loadedModules_, rhs.loadedModules_);

    return *this;
}

ShaderModuleFactory::~ShaderModuleFactory()
{
    for (auto const& module : loadedModules_) {
        table_->vkDestroyShaderModule(device_->Handle(), module.handle_, nullptr);
    }
}

ShaderModule* ShaderModuleFactory::LoadModule(ShaderModuleDesc const& desc)
{
    VkShaderModule result = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pCode = nullptr;
    info.flags = VK_FLAGS_NONE;
    info.codeSize = 0;
    info.pCode = nullptr;
    VK_ASSERT(table_->vkCreateShaderModule(device_->Handle(), &info, nullptr, &result));
}

void ShaderModuleFactory::UnloadModule(ShaderModule* module)
{
    std::size_t const modulesCount = loadedModules_.size();
    for (auto i = 0u; i < modulesCount; ++i) {
        auto localHandle = loadedModules_[i].handle_;
        if (localHandle == module->handle_) {
            table_->vkDestroyShaderModule(device_->Handle(), localHandle, nullptr);
            loadedModules_.erase(loadedModules_.begin() + i);
            return;
        }
    }
}

}