#include "WorkersControlSystem.hpp"
#include <utility>

namespace VKW
{

WorkersControlSystem::WorkersControlSystem()
    : table_{ nullptr }
    , device_{ nullptr }
{
}

WorkersControlSystem::WorkersControlSystem(WorkersControlSystemDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
    std::uint32_t constexpr QUEUE_TYPES_SIZE = 3;

    
    
    std::uint32_t queueIndecies[QUEUE_TYPES_SIZE] = {
        FindFamilyIndex(device_, DeviceQueueType::GRAPHICS, desc.graphicsQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::COMPUTE, desc.computeQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::TRANSFER, desc.transferQueueCount_)
    };

    std::uint32_t const queueCounts[QUEUE_TYPES_SIZE] = {
        desc.graphicsQueueCount_,
        desc.computeQueueCount_,
        desc.transferQueueCount_
    };

    WorkerType workerGroupTypes[QUEUE_TYPES_SIZE] = {
        WorkerType::GRAPHICS,
        WorkerType::COMPUTE,
        WorkerType::TRANSFER
    };

    VKW::WorkerGroup* workerGroups[QUEUE_TYPES_SIZE] = {
        &graphicsGroup_,
        &computeGroup_,
        &transferGroup_
    };


    for (auto i = 0u; i < QUEUE_TYPES_SIZE; ++i) {
        WorkerGroupDesc workerGroupDesc;
        workerGroupDesc.table_ = desc.table_;
        workerGroupDesc.device_ = desc.device_;
        workerGroupDesc.type_ = workerGroupTypes[i];
        workerGroupDesc.familyIndex_ = queueIndecies[i];
        workerGroupDesc.workersCount_ = queueCounts[i];

        *workerGroups[i] = VKW::WorkerGroup{ workerGroupDesc };
    }
}

WorkersControlSystem::WorkersControlSystem(WorkersControlSystem&& rhs)
{
    operator=(std::move(rhs));
}

WorkersControlSystem& WorkersControlSystem::operator=(WorkersControlSystem&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    
    std::swap(graphicsGroup_, rhs.graphicsGroup_);
    std::swap(computeGroup_, rhs.computeGroup_);
    std::swap(transferGroup_, rhs.transferGroup_);

    return *this;
}

WorkersControlSystem::~WorkersControlSystem()
{

}

Worker* WorkersControlSystem::GetWorker(WorkerType type)
{
    return nullptr;
}

std::uint32_t WorkersControlSystem::FindFamilyIndex(Device const* device, DeviceQueueType type, std::uint32_t requiredCount)
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