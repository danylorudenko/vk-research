#include "WorkersProvider.hpp"
#include "../Tools.hpp"
#include <utility>

namespace VKW
{

WorkersProvider::WorkersProvider()
    : table_{ nullptr }
    , device_{ nullptr }
{
}

WorkersProvider::WorkersProvider(WorkersProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    std::uint32_t constexpr QUEUE_TYPES_SIZE = 3;

    
    
    std::uint32_t queueIndecies[QUEUE_TYPES_SIZE] = {
        FindFamilyIndex(device_, DeviceQueueType::GRAPHICS, desc.graphicsQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::COMPUTE, desc.computeQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::TRANSFER, desc.transferQueueCount_)
    };

    WorkerType workerGroupTypes[QUEUE_TYPES_SIZE] = {
        WorkerType::GRAPHICS,
        WorkerType::COMPUTE,
        WorkerType::TRANSFER
    };

    std::uint32_t const queueCounts[QUEUE_TYPES_SIZE] = {
        desc.graphicsQueueCount_,
        desc.computeQueueCount_,
        desc.transferQueueCount_
    };

    std::unique_ptr<WorkerGroup>* workerGroups[QUEUE_TYPES_SIZE] = {
        &graphicsGroup_,
        &computeGroup_,
        &transferGroup_
    };


    for (auto i = 0u; i < QUEUE_TYPES_SIZE; ++i) {
        if (queueCounts[i] > 0) {
            WorkerGroupDesc workerGroupDesc;
            workerGroupDesc.table_ = desc.table_;
            workerGroupDesc.device_ = desc.device_;
            workerGroupDesc.type_ = workerGroupTypes[i];
            workerGroupDesc.familyIndex_ = queueIndecies[i];
            workerGroupDesc.workersCount_ = queueCounts[i];

            *workerGroups[i] = std::make_unique<VKW::WorkerGroup>(workerGroupDesc);
        }
    }
}

WorkersProvider::WorkersProvider(WorkersProvider&& rhs)
{
    operator=(std::move(rhs));
}

WorkersProvider& WorkersProvider::operator=(WorkersProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    
    std::swap(graphicsGroup_, rhs.graphicsGroup_);
    std::swap(computeGroup_, rhs.computeGroup_);
    std::swap(transferGroup_, rhs.transferGroup_);

    return *this;
}

WorkersProvider::~WorkersProvider()
{
    VK_ASSERT(table_->vkDeviceWaitIdle(device_->Handle()));
}

Worker* WorkersProvider::GetWorker(WorkerType type, std::uint32_t index)
{
    switch (type) {
    case WorkerType::GRAPHICS:
        return graphicsGroup_->GetWorker(index);
    case WorkerType::COMPUTE:
        return computeGroup_->GetWorker(index);
    case WorkerType::TRANSFER:
        return transferGroup_->GetWorker(index);
    default:
        return nullptr;
    }
}

std::uint32_t WorkersProvider::FindFamilyIndex(Device const* device, DeviceQueueType type, std::uint32_t requiredCount)
{
    std::uint32_t result = std::numeric_limits<std::uint32_t>::max();

    if (requiredCount == 0) {
        return result;
    }

    auto const familyCount = device->QueueFamilyCount();
    for (auto i = 0u; i < familyCount; ++i) {
        auto const& familyDesc = device->GetQueueFamily(i);
        if (familyDesc.type_ == type && familyDesc.count_ >= requiredCount) {
            result = familyDesc.familyIndex_;
            break;
        }
    }

    assert(result != std::numeric_limits<std::uint32_t>::max() && "Couldn't find queue family index for WorkerGroup");

    return result;
}


}