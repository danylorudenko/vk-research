#include "Worker.hpp"

#include <utility>

namespace VKW
{

Worker::Worker()
{
}

Worker::Worker(WorkerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , type_{ desc.type_ }
    , queue_{ VK_NULL_HANDLE }
{
}

Worker::Worker(Worker&& rhs)
{
    operator=(std::move(rhs));
}

Worker& Worker::operator=(Worker&& rhs)
{

    return *this;
}

Worker::~Worker()
{
}

}