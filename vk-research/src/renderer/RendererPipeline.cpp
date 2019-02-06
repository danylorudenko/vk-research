#include "RendererPipeline.hpp"
#include <utility>

namespace Render
{

Pipeline::Pipeline()
{

}

Pipeline::Pipeline(VKW::PipelineHandle handle)
    : pipelineHandle_{ handle }
{

}

Pipeline::Pipeline(Pipeline&& rhs)
{
    operator=(std::move(rhs));
}

Pipeline& Pipeline::operator=(Pipeline&& rhs)
{
    std::swap(pipelineHandle_, rhs.pipelineHandle_);

    return *this;
}

VKW::PipelineHandle Pipeline::Handle() const
{
    return pipelineHandle_;
}

Pipeline::~Pipeline()
{

}

}