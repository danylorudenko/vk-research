#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "Pass.hpp"

namespace VKW
{
    class ImportTable;
    class Device;
    class ResourceRendererProxy;
    class PipelineFactory;
    class DescriptorLayoutController;
}

namespace Render
{

class Root;

class CustomTempBlurPass
    : public BasePass
{
public:
    CustomTempBlurPass();
    CustomTempBlurPass(CustomTempBlurPass&& rhs);
    CustomTempBlurPass& operator=(CustomTempBlurPass&& rhs);
    ~CustomTempBlurPass();

private:
    Root* root_;

    VKW::ImportTable* table_;
    VKW::Device* device_;

    VKW::ResourceRendererProxy* resourceProxy_;
    VKW::PipelineFactory* pipelineFactory_;
    VKW::DescriptorLayoutController* descriptorLayoutController_;
};

}

