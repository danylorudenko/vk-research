#include "Worker.hpp"

#include <utility>

#include <vk_interface\Tools.hpp>
#include <vk_interface\worker\WorkerGroup.hpp>

namespace VKW
{

Worker::Worker()
    : table_{ nullptr }
    , device_{ nullptr }
    , parentGroup_{ nullptr }
    , type_{ WorkerType::NONE }
    , queue_{ VK_NULL_HANDLE }
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
    std::swap(executionFrames_, rhs.executionFrames_);

    return *this;
}

WorkerFrameCommandReciever Worker::StartExecutionFrame(std::uint32_t contextId)
{    
    WorkerFrame& currentFrame = executionFrames_[contextId];
    return currentFrame.Begin();
}

void Worker::EndExecutionFrame(std::uint32_t contextId)
{
    WorkerFrame& currentFrame = executionFrames_[contextId];
    currentFrame.End();
}

WorkerFrameCompleteSemaphore Worker::ExecuteFrame(std::uint32_t contextId, VkSemaphore waitSemaphore, bool signaling)
{
    WorkerFrame& currentFrame = executionFrames_[contextId];
    return currentFrame.Execute(queue_, waitSemaphore, signaling);
}

VkQueue Worker::QueueHandle() const
{
    return queue_;
}

Worker::~Worker()
{
    // WorkersProvider is responsible for waiting on the destruction
    /*for (auto& frame : executionFrames_) {
        frame.WaitForFence();
    }*/
}

}