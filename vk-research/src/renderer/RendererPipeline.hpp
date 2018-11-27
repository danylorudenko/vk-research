#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\Pipeline.hpp"

namespace Render
{

class Pipeline
    : public NonCopyable
{
public:
    Pipeline();
    Pipeline(VKW::PipelineHandle handle);
    Pipeline(Pipeline&& rhs);
    Pipeline& operator=(Pipeline&& rhs);
    ~Pipeline();

private:
    VKW::PipelineHandle pipelineHandle_;
};

}