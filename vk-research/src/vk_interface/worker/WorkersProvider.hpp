#pragma once

#include <utility>

#include "..\..\class_features\NonCopyable.hpp"

#include "WorkerGroup.hpp"
#include "..\Device.hpp"

namespace VKW
{

struct WorkersProviderDesc
{
    ImportTable* table_;
    Device* device_;

    std::uint32_t graphicsQueueCount_;
    std::uint32_t computeQueueCount_;
    std::uint32_t transferQueueCount_;
};

class WorkersProvider
    : public NonCopyable
{
public:
    WorkersProvider();
    WorkersProvider(WorkersProviderDesc const& desc);

    WorkersProvider(WorkersProvider&& rhs);
    WorkersProvider& operator=(WorkersProvider&& rhs);

    ~WorkersProvider();

    Worker* GetWorker(WorkerType type, std::uint32_t index);

private:
    static std::uint32_t FindFamilyIndex(Device const* device, DeviceQueueType type, std::uint32_t requiredCount);

private:
    ImportTable* table_;
    Device* device_;

    std::unique_ptr<WorkerGroup> graphicsGroup_;
    std::unique_ptr<WorkerGroup> computeGroup_;
    std::unique_ptr<WorkerGroup> transferGroup_;
};

}