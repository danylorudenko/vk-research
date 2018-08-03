#include "WorkerFrame.h"

#include <utility>
#include "..\Tools.hpp"

namespace VKW
{

WorkerFrame::WorkerFrame() 
    : table_{ nullptr }
    , device_{ nullptr }
    , commandBuffer_{ VK_NULL_HANDLE }
    , fence_{ VK_NULL_HANDLE }
    , inExecution_{ false }
{
}

WorkerFrame::WorkerFrame(WorkerFrameDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , commandBuffer_{ desc.commandBuffer_ }
    , fence_{ VK_NULL_HANDLE }
    , inExecution_{ false }
{
    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FLAGS_NONE;

    VK_ASSERT(table_->vkCreateFence(device_->Handle(), &fenceInfo, nullptr, &fence_));
}

WorkerFrame::WorkerFrame(WorkerFrame&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , commandBuffer_{ VK_NULL_HANDLE }
    , fence_{ VK_NULL_HANDLE }
    , inExecution_{ false }
{
    operator=(std::move(rhs));
}

WorkerFrame& WorkerFrame::operator=(WorkerFrame&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(commandBuffer_, rhs.commandBuffer_);
    std::swap(fence_, rhs.fence_);
    std::swap(inExecution_, rhs.inExecution_);

    return *this;
}

VkCommandBuffer WorkerFrame::Begin()
{
    WaitForFence();
    inExecution_ = false;
    
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = VK_FLAGS_NONE;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VK_ASSERT(table_->vkBeginCommandBuffer(commandBuffer_, &beginInfo));

    return commandBuffer_;
}

void WorkerFrame::End()
{
    table_->vkEndCommandBuffer(commandBuffer_);
}

void WorkerFrame::Execute(VkQueue queue)
{
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer_;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    ResetFence();
    inExecution_ = true;

    VK_ASSERT(table_->vkQueueSubmit(queue, 1, &submitInfo, fence_));
}

void WorkerFrame::WaitForFence()
{
    if (inExecution_) {
        VK_ASSERT(table_->vkWaitForFences(device_->Handle(), 1, &fence_, false, std::numeric_limits<std::uint64_t>::max()));
    }
}

void WorkerFrame::ResetFence()
{
    VK_ASSERT(table_->vkResetFences(device_->Handle(), 1, &fence_));
}

WorkerFrame::~WorkerFrame()
{
    if (fence_ != VK_NULL_HANDLE) {
        table_->vkDestroyFence(device_->Handle(), fence_, nullptr);
    }
}

}