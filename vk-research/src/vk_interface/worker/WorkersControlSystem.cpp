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

    std::uint32_t const queueIndecies[QUEUE_TYPES_SIZE] = {
        desc.graphicsQueueIndex_,
        desc.computeQueueIndex_,
        desc.transferQueueIndex_
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


}