#pragma once

#include "..\Device.hpp"

#include <vulkan/vulkan.h>

namespace VKW
{


struct WorkerFrameDesc
{
    ImportTable* table_;
    Device* device_;
    
    VkCommandBuffer commandBuffer_;
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

    VkCommandBuffer Begin();
    void End();
    void Execute();

    void WaitAndResetFence();
    void WaitForFence();
    void ResetFence();

    
private:
    ImportTable* table_;
    Device* device_;
    
    VkCommandBuffer commandBuffer_;
    VkFence fence_;
    bool inExecution_;


};

}