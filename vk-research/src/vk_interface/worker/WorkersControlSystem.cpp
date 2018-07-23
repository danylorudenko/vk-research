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
    auto const& familyProperties = device_->Properties().queueFamilyProperties;
    for (auto i = 0u; i < desc.graphicsQueueCount_; ++i) {
        VkQueue queueHandle = VK_NULL_HANDLE;
        table_->vkGetDeviceQueue(device_->Handle(), desc.graphicsQueueIndex_, i, &queueHandle);
        assert(queueHandle != VK_NULL_HANDLE && "Can't get device queue.");
        
        WorkerDesc workerDesc;
        workerDesc.table_ = desc.table_;
        workerDesc.device_ = desc.device_;
        workerDesc.queue_ = queueHandle;
        workerDesc.type_ = WorkerType::GRAPHICS;
        workerDesc.bufferingCount_ = 1;
        
        graphicsWorkers_.emplace_back(workerDesc);
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
    
    std::swap(graphicsWorkers_, rhs.graphicsWorkers_);
    std::swap(computeWorkers_, rhs.computeWorkers_);
    std::swap(transferWorkers_, rhs.transferWorkers_);

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