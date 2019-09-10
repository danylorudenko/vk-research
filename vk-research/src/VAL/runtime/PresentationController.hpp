#pragma once

#include <vector>
#include <vulkan\vulkan.h>
#include "..\..\class_features\NonCopyable.hpp"

namespace VAL
{

class ImportTable;
class Device;
class Swapchain;
class Worker;
struct WorkerFrameCompleteSemaphore;


struct PresentationContext
{
    std::uint32_t contextId_;
    VkSemaphore contextPresentationCompleteSemaphore_;
};


struct PresentationControllerDesc
{
    ImportTable* table_;
    Device* device_;
    Swapchain* swapchain_;
    Worker* presentationWorker_;
};

class PresentationController
    : public NonCopyable
{
public:
    PresentationController();
    PresentationController(PresentationControllerDesc const& desc);

    PresentationController(PresentationController&& rhs);
    PresentationController& operator=(PresentationController&& rhs);

    ~PresentationController();

    PresentationContext AcquireNewPresentationContext();
    void PresentContextId(std::uint32_t contextId, WorkerFrameCompleteSemaphore frameRenderingCompleteSemaphore);



private:
    ImportTable* table_;
    Device* device_;
    Swapchain* swapchain_;
    Worker* presentationWorker_;

    std::uint64_t nextSemaphore_;
    std::vector<VkSemaphore> semaphoresQueue_;
};

}