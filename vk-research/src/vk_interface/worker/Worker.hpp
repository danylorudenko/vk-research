#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "..\..\class_features\NonCopyable.hpp"

#include "..\Device.hpp"

namespace VKW
{

enum class WorkerType
{
    GRAPHICS,
    COMPUTE,
    TRANSFER
};


struct WorkerDesc
{
    ImportTable* table_;
    Device* device_;
    VkQueue queue_;
    WorkerType type_;
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

private:
    ImportTable* table_;
    Device* device_;
    
    WorkerType type_;
    VkQueue queue_;

    std::vector<VkCommandBuffer> commandBuffers_;

};

}