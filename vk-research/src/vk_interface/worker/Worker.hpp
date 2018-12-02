#pragma once

#include <cstdint>
#include <vector>
#include <vulkan\vulkan.h>

#include "..\..\class_features\NonCopyable.hpp"

#include "..\Device.hpp"
#include "WorkerFrame.hpp"

namespace VKW
{


class WorkerGroup;


enum class WorkerType
{
    NONE,
    GRAPHICS_PRESENT,
    COMPUTE,
    TRANSFER
};


struct WorkerDesc
{
    ImportTable* table_;
    Device* device_;
    WorkerGroup* parentGroup_;

    WorkerType type_;
    std::uint32_t queueFamilyIndex_;
    std::uint32_t queueIndex_;

    std::uint32_t bufferingCount_;
};



class Worker
    : public NonCopyable
{
public:
    Worker();
    Worker(WorkerDesc const& desc);

    Worker(Worker&& rhs);
    Worker& operator=(Worker&& rhs);

    ~Worker();

    VkQueue QueueHandle() const;

    WorkerFrameCommandReciever StartNextExecutionFrame();
    void EndCurrentExecutionFrame();
    WorkerFrameCompleteSemaphore ExecuteCurrentFrame();


private:
    ImportTable* table_;
    Device* device_;
    WorkerGroup* parentGroup_;
    
    WorkerType type_;
    VkQueue queue_;

    std::uint64_t currentExecutionFrame_;
    std::vector<WorkerFrame> executionFrames_;

};

}