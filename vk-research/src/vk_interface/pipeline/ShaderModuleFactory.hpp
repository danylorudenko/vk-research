#pragma once

#include "ShaderModule.hpp"
#include <vector>

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

    ShaderModuleHandle LoadModule(ShaderModuleDesc const& desc);
    void UnloadModule(ShaderModuleHandle module);

    ShaderModule* AccessModule(ShaderModuleHandle handle) const;

    ~ShaderModuleFactory();

private:
    ImportTable* table_;
    Device* device_;
    IOManager* ioManager_;

    std::vector<ShaderModule*> loadedModules_;
};

}