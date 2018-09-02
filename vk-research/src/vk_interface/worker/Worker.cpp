#include "Worker.hpp"
#include "WorkerGroup.hpp"

#include <utility>

#include "../Tools.hpp"

namespace VKW
{

Worker::Worker()
    : table_{ nullptr }
    , device_{ nullptr }
    , parentGroup_{ nullptr }
    , type_{ WorkerType::NONE }
    , queue_{ VK_NULL_HANDLE }
    , currentExecutionFrame_{ 0 }
{
}

Worker::Worker(WorkerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , parentGroup_{ desc.parentGroup_ }
    , type_{ desc.type_ }
    , queue_{ VK_NULL_HANDLE }
    , currentExecutionFrame_{ 0 }
{
    table_->vkGetDeviceQueue(device_->Handle(), desc.queueFamilyIndex_, desc.queueIndex_, &queue_);
    assert(queue_ != VK_NULL_HANDLE && "Can't get device queue.");


    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(desc.bufferingCount_);

    parentGroup_->AllocCommandBuffers(desc.bufferingCount_, commandBuffers.data());

    for (auto i = 0u; i < desc.bufferingCount_; ++i) {
        WorkerFrameDesc frameDesc;
        frameDesc.table_ = table_;
        frameDesc.device_ = device_;
        frameDesc.commandBuffer_ = commandBuffers[i];
        executionFrames_.emplace_back(frameDesc);
    }
    
}

Worker::Worker(Worker&& rhs)
{
    operator=(std::move(rhs));
}

Worker& Worker::operator=(Worker&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(parentGroup_, rhs.parentGroup_);
    std::swap(type_, rhs.type_);
    std::swap(queue_, rhs.queue_);
    std::swap(currentExecutionFrame_, rhs.currentExecutionFrame_);
    std::swap(executionFrames_, rhs.executionFrames_);

    return *this;
}

VkCommandBuffer Worker::StartNextExecutionFrame()
{
    currentExecutionFrame_ = (currentExecutionFrame_ + 1) % executionFrames_.size();
    
    WorkerFrame& currentFrame = executionFrames_[currentExecutionFrame_];
    return currentFrame.Begin();
}

void Worker::EndCurrentExecutionFrame()
{
    WorkerFrame& currentFrame = executionFrames_[currentExecutionFrame_];
    currentFrame.End();
}

void Worker::ExecuteCurrentFrame()
{
    WorkerFrame& currentFrame = executionFrames_[currentExecutionFrame_];
    currentFrame.Execute(queue_);
}

Worker::~Worker()
{
    // WorkersProvider is responsible for waiting on the destruction
    /*for (auto& frame : executionFrames_) {
        frame.WaitForFence();
    }*/
}

}