#include "ShaderModuleFactory.hpp"
#include <utility>
#include "../ImportTable.hpp"
#include "../Device.hpp"
#include "../Tools.hpp"
#include "../../io/IOManager.hpp"
#include "../../memory/ByteBuffer.hpp"

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

ShaderModuleHandle ShaderModuleFactory::LoadModule(ShaderModuleDesc const& desc)
{
    ByteBuffer buffer{ 8192 };
    auto dataSize = ioManager_->ReadFileToBuffer(desc.shaderPath_, buffer);
    
    VkShaderModule result = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext= nullptr;
    info.flags = VK_FLAGS_NONE;
    info.codeSize = dataSize;
    info.pCode = buffer.As<std::uint32_t const*>();
    VK_ASSERT(table_->vkCreateShaderModule(device_->Handle(), &info, nullptr, &result));

    loadedModules_.emplace_back(result);
    return { static_cast<std::uint32_t>(loadedModules_.size()) - 1 };
}

void ShaderModuleFactory::UnloadModule(ShaderModuleHandle module)
{
    auto& deletedModule = loadedModules_[module.id_];
    
    std::size_t const modulesCount = loadedModules_.size();
    for (auto i = 0u; i < modulesCount; ++i) {
        auto localHandle = loadedModules_[i].handle_;
        if (localHandle == deletedModule.handle_) {
            table_->vkDestroyShaderModule(device_->Handle(), localHandle, nullptr);
            loadedModules_.erase(loadedModules_.begin() + i);
            return;
        }
    }
}

ShaderModule const& ShaderModuleFactory::AccessModule(ShaderModuleHandle handle) const
{
    return loadedModules_[handle.id_];
}

}