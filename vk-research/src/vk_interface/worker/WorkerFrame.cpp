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
{
    operator=(std::move(rhs));
}

WorkerFrame& WorkerFrame::operator=(WorkerFrame&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(commandBuffer_, rhs.commandBuffer_);
    std::swap(fence_, rhs.fence_);

    return *this;
}

VkCommandBuffer WorkerFrame::Begin()
{

}

void WorkerFrame::End()
{

}

void WorkerFrame::Execute()
{

}

void WorkerFrame::WaitForFence()
{
    table_->vkWaitForFences(device_->Handle(), 1, &fence_, false, std::numeric_limits<std::uint64_t>::max());
}

void WorkerFrame::ResetFence()
{
    table_->vkResetFences(device_->Handle(), 1, &fence_);
}

void WorkerFrame::WaitAndResetFence()
{
    WaitForFence();
    ResetFence();
}

WorkerFrame::~WorkerFrame()
{
    table_->vkDestroyFence(device_->Handle(), fence_, nullptr);
}

}