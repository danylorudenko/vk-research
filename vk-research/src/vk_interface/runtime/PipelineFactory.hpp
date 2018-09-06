#pragma once

#include "../../class_features/NonCopyable.hpp"

namespace VKW
{

class ImportTable;
class Device;


struct PipelineFactoryDesc
{
    ImportTable* table_;
    Device* device_;
};

class PipelineFactory
{
    PipelineFactory();
    PipelineFactory(PipelineFactoryDesc const& desc);

    PipelineFactory(PipelineFactory&& rhs);
    PipelineFactory& operator=(PipelineFactory&& rhs);

    ~PipelineFactory();

private:
    ImportTable* table_;
    Device* device_;
};

}