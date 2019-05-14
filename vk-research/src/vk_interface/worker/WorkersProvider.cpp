#include "WorkersProvider.hpp"
#include "..\Tools.hpp"
#include <utility>

namespace VKW
{

WorkersProvider::WorkersProvider()
    : table_{ nullptr }
    , device_{ nullptr }
    , graphicsPresentGroup_{ nullptr }
    , computeGroup_{ nullptr }
    , transferGroup_{ nullptr }
{
}

WorkersProvider::WorkersProvider(WorkersProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , graphicsPresentGroup_{ nullptr }
    , computeGroup_{ nullptr }
    , transferGroup_{ nullptr }
{
    std::uint32_t constexpr QUEUE_TYPES_SIZE = 3;

    
    
    std::uint32_t const queueIndecies[QUEUE_TYPES_SIZE] = {
        FindFamilyIndex(device_, DeviceQueueType::GRAPHICS_PRESENT, desc.graphicsPresentQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::COMPUTE, desc.computeQueueCount_),
        FindFamilyIndex(device_, DeviceQueueType::TRANSFER, desc.transferQueueCount_)
    };

    WorkerType const workerGroupTypes[QUEUE_TYPES_SIZE] = {
        WorkerType::GRAPHICS_PRESENT,
        WorkerType::COMPUTE,
        WorkerType::TRANSFER
    };

    std::uint32_t const queueCounts[QUEUE_TYPES_SIZE] = {
        desc.graphicsPresentQueueCount_,
        desc.computeQueueCount_,
        desc.transferQueueCount_
    };

    std::unique_ptr<WorkerGroup> workerGroups[QUEUE_TYPES_SIZE];


    for (auto i = 0u; i < QUEUE_TYPES_SIZE; ++i) {
        if (queueCounts[i] > 0) {
            WorkerGroupDesc workerGroupDesc;
            workerGroupDesc.table_ = desc.table_;
            workerGroupDesc.device_ = desc.device_;
            workerGroupDesc.type_ = workerGroupTypes[i];
            workerGroupDesc.familyIndex_ = queueIndecies[i];
            workerGroupDesc.workersCount_ = queueCounts[i];
            workerGroupDesc.bufferingCount_ = desc.bufferingCount_;

            workerGroups[i] = std::make_unique<VKW::WorkerGroup>(workerGroupDesc);
        }
    }


    graphicsPresentGroup_ = std::move(workerGroups[0]);
    computeGroup_ = std::move(workerGroups[1]);
    transferGroup_ = std::move(workerGroups[2]);

    presentWorker_ = graphicsPresentGroup_->GetWorker(0);
}

WorkersProvider::WorkersProvider(WorkersProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , graphicsPresentGroup_{ nullptr }
    , computeGroup_{ nullptr }
    , transferGroup_{ nullptr }
{
    operator=(std::move(rhs));
}

WorkersProvider& WorkersProvider::operator=(WorkersProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    
    std::swap(graphicsPresentGroup_, rhs.graphicsPresentGroup_);
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
    case WorkerType::GRAPHICS_PRESENT:
        return graphicsPresentGroup_->GetWorker(index);
    case WorkerType::COMPUTE:
        return computeGroup_->GetWorker(index);
    case WorkerType::TRANSFER:
        return transferGroup_->GetWorker(index);
    default:
        return nullptr;
    }
}

Worker* WorkersProvider::PresentWorker() const
{
    return presentWorker_;
}

std::uint32_t WorkersProvider::FindFamilyIndex(Device const* device, DeviceQueueType type, std::uint32_t requiredCount)
{
    std::uint32_t constexpr INVALID_RESULT = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t result = INVALID_RESULT;

    if (requiredCount == 0) {
        return result;
    }

    bool const presentRequired = type == DeviceQueueType::GRAPHICS_PRESENT;

    std::uint32_t const familyCount = device->QueueFamilyCount();
    for (std::uint32_t i = 0u; i < familyCount; ++i) {
        VKW::DeviceQueueFamilyInfo const& familyDesc = device->GetQueueFamily(i);

        //if presentation is required on this type of family, skip family in case it doesn't support presentation
        if (presentRequired && !familyDesc.presentationSupported_) {
            continue;
        }

        if (familyDesc.type_ == type && 
            familyDesc.count_ >= requiredCount) {
            result = familyDesc.familyIndex_;
            break;
        }
    }

    assert(result != INVALID_RESULT && "Couldn't find queue family index for WorkerGroup");

    return result;
}


}