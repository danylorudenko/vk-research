#pragma once

#include <unordered_set>
#include <vk_interface\pipeline\ShaderModule.hpp>

class IOManager;

namespace VKW
{

class Device;
class ImportTable;

struct ShaderModuleFactoryDesc
{
    ImportTable* table_;
    Device* device_;
    IOManager* ioManager_;
};

struct ShaderModuleDesc
{
    char const* shaderPath_;
    ShaderModuleType type_;
};

class ShaderModuleFactory
{
public:
    ShaderModuleFactory();
    ShaderModuleFactory(ShaderModuleFactoryDesc const& desc);

    ShaderModuleFactory(ShaderModuleFactory&& rhs);
    ShaderModuleFactory& operator=(ShaderModuleFactory&& rhs);

    ShaderModule* LoadModule(ShaderModuleDesc const& desc);
    void UnloadModule(ShaderModule* module);

    ~ShaderModuleFactory();

private:
    ImportTable* table_;
    Device* device_;
    IOManager* ioManager_;

    std::unordered_set<ShaderModule*> loadedModules_;
};

}