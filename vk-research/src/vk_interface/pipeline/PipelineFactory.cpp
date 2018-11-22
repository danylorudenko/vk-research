#include "PipelineFactory.hpp"

#include <utility>

namespace VKW
{

PipelineFactory::PipelineFactory()
    : table_{ nullptr }
    , device_{ nullptr }
{

}

PipelineFactory::PipelineFactory(PipelineFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{

}

PipelineFactory::PipelineFactory(PipelineFactory&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

PipelineFactory& PipelineFactory::operator=(PipelineFactory&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(pipelines_, rhs.pipelines_);

    return *this;
}

PipelineFactory::~PipelineFactory()
{

}


}