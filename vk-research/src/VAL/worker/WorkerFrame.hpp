#pragma once

#include "..\Device.hpp"

#include <vulkan\vulkan.h>

namespace VAL
{


struct WorkerFrameDesc
{
    ImportTable* table_;
    Device* device_;
    
    VkCommandBuffer commandBuffer_;
};

struct WorkerFrameCommandReciever
{
    VkCommandBuffer commandBuffer_;
};

struct WorkerFrameCompleteSemaphore
{
    VkSemaphore frameCompleteSemaphore_;
};

class WorkerFrame
    : public NonCopyable
{
public:
    WorkerFrame();
    WorkerFrame(WorkerFrameDesc const& desc);

    WorkerFrame(WorkerFrame&& rhs);
    WorkerFrame& operator=(WorkerFrame&& rhs);

    ~WorkerFrame();

    WorkerFrameCommandReciever Begin();
    void End();
    WorkerFrameCompleteSemaphore Execute(VkQueue queue, VkSemaphore waitSemaphore, bool signaling);

    void WaitForFence();
    void ResetFence();

    
private:
    ImportTable* table_;
    Device* device_;
    
    VkCommandBuffer commandBuffer_;
    VkFence fence_;
    VkSemaphore frameCompleteSemaphore_;

    bool inExecution_;


};

}