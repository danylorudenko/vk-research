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