#include "ShaderModuleFactory.hpp"

#include <utility>

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Tools.hpp>
#include <memory\ByteBuffer.hpp>
#include <io\IOManager.hpp>

namespace VKW
{

char constexpr DEFAULT_SHADER_ENTRY_POINT[] = "main";

ShaderModuleFactory::ShaderModuleFactory()
    : table_{ nullptr }
    , device_{ nullptr }
    , ioManager_{ nullptr }
{
}

ShaderModuleFactory::ShaderModuleFactory(ShaderModuleFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , ioManager_{ desc.ioManager_ }
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
    for (auto const& shaderModule : loadedModules_) {
        table_->vkDestroyShaderModule(device_->Handle(), shaderModule->handle_, nullptr);
        delete shaderModule;
    }
}

ShaderModule* ShaderModuleFactory::LoadModule(ShaderModuleDesc const& desc)
{
    ByteBuffer buffer{ 8192 };
    auto dataSize = ioManager_->ReadFileToBuffer(desc.shaderPath_, buffer);
    
    VkShaderModule vkModule = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext= nullptr;
    info.flags = VK_FLAGS_NONE;
    info.codeSize = dataSize;
    info.pCode = buffer.As<std::uint32_t const*>();
    ERR_GUARD_VK(table_->vkCreateShaderModule(device_->Handle(), &info, nullptr, &vkModule));

    auto* resultModule = new ShaderModule{ vkModule };
    resultModule->entryPoint_ = DEFAULT_SHADER_ENTRY_POINT;
    resultModule->type_ = desc.type_;
    loadedModules_.emplace(resultModule);

    return resultModule;
}

void ShaderModuleFactory::UnloadModule(ShaderModule* module)
{
    auto moduleIt = loadedModules_.find(module);
    assert(moduleIt != loadedModules_.end() && "Can't delete this shader module.");

    table_->vkDestroyShaderModule(device_->Handle(), module->handle_, nullptr);
    loadedModules_.erase(module);

    delete module;
}

}