#pragma once

#include <vector>
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

class ShaderModuleFactory
{
public:
    ShaderModuleFactory();
    ShaderModuleFactory(ShaderModuleFactoryDesc const& desc);

    ShaderModuleFactory(ShaderModuleFactory&& rhs);
    ShaderModuleFactory& operator=(ShaderModuleFactory&& rhs);

    ShaderModule* LoadModule(char const* shaderPath, ShaderModuleType type);
    void UnloadModule(ShaderModule* module);

    ~ShaderModuleFactory();

private:
    ImportTable* table_;
    Device* device_;
    IOManager* ioManager_;

    std::vector<ShaderModule*> loadedModules_;
};

}