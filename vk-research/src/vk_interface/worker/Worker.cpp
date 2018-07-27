#include "Worker.hpp"
#include "WorkerGroup.hpp"

#include <utility>

namespace VKW
{

Worker::Worker()
{
}

Worker::Worker(WorkerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , parentGroup_{ desc.parentGroup_ }
    , type_{ desc.type_ }
    , queue_{ VK_NULL_HANDLE }
{
    table_->vkGetDeviceQueue(device_->Handle(), desc.queueFamilyIndex_, desc.queueIndex_, &queue_);
    assert(queue_ != VK_NULL_HANDLE && "Can't get device queue.");

    //commandBuffers_.resize(desc.bufferingCount_);
    //parentGroup_->AllocCommandBuffers(desc.bufferingCount_, commandBuffers_.data());
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