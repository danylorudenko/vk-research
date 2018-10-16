#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include "../../class_features/NonCopyable.hpp"

namespace VKW
{

class ImportTable;
class Device;
class Swapchain;
class Worker;

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

    std::uint32_t AcquireNewContextId();

    ~PresentationController();


private:
    ImportTable* table_;
    Device* device_;
    Swapchain* swapchain_;
    Worker* presentationWorker_;

    VkSemaphore imageAcquireSemaphore_;
};

}