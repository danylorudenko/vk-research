#include "ShaderModuleFactory.hpp"
#include <utility>
#include "..\ImportTable.hpp"
#include "..\Device.hpp"
#include "..\Tools.hpp"
#include "..\..\io\IOManager.hpp"
#include "..\..\memory\ByteBuffer.hpp"

namespace VAL
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

ShaderModuleHandle ShaderModuleFactory::LoadModule(ShaderModuleDesc const& desc)
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
    VK_ASSERT(table_->vkCreateShaderModule(device_->Handle(), &info, nullptr, &vkModule));

    auto* resultModule = new ShaderModule{ vkModule };
    resultModule->entryPoint_ = DEFAULT_SHADER_ENTRY_POINT;
    resultModule->type_ = desc.type_;
    loadedModules_.emplace_back(resultModule);

    return { resultModule };
}

void ShaderModuleFactory::UnloadModule(ShaderModuleHandle module)
{
    auto deletedModule = module.handle_;
    
    std::size_t const modulesCount = loadedModules_.size();
    for (auto i = 0u; i < modulesCount; ++i) {
        auto localHandle = loadedModules_[i]->handle_;
        if (localHandle == deletedModule->handle_) {
            table_->vkDestroyShaderModule(device_->Handle(), localHandle, nullptr);
            loadedModules_.erase(loadedModules_.begin() + i);
            delete deletedModule;
            return;
        }
    }
}

ShaderModule* ShaderModuleFactory::AccessModule(ShaderModuleHandle handle) const
{
    return handle.handle_;
}

}