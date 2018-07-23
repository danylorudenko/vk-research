#pragma once

#include "..\..\class_features\NonCopyable.hpp"

#include "WorkerGroup.hpp"
#include "..\Device.hpp"

namespace VKW
{

struct WorkersControlSystemDesc
{
    ImportTable* table_;
    Device* device_;

    std::uint32_t graphicsQueueIndex_;
    std::uint32_t graphicsQueueCount_;

    std::uint32_t computeQueueIndex_;
    std::uint32_t computeQueueCount_;

    std::uint32_t transferQueueIndex_;
    std::uint32_t transferQueueCount_;
};

class WorkersControlSystem
    : public NonCopyable
{
public:
    WorkersControlSystem();
    WorkersControlSystem(WorkersControlSystemDesc const& desc);

    WorkersControlSystem(WorkersControlSystem&& rhs);
    WorkersControlSystem& operator=(WorkersControlSystem&& rhs);

    ~WorkersControlSystem();

    Worker* GetWorker(WorkerType type);

private:
    ImportTable* table_;
    Device* device_;

    WorkerGroup graphicsGroup_;
    WorkerGroup computeGroup_;
    WorkerGroup transferGroup_;
};

}