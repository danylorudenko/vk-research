#include "WorkerGroup.hpp"

#include "..\Tools.hpp"

#include <limits>

namespace VKW
{

WorkerGroup::WorkerGroup()
    : table_{ nullptr }
    , device_{ nullptr }
    , familyIndex_{ std::numeric_limits<std::uint32_t>::max() }
    , type_{ WorkerType::NONE }
    , commandPool_{ VK_NULL_HANDLE }
{
}

WorkerGroup::WorkerGroup(WorkerGroupDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , familyIndex_{ desc.familyIndex_ }
    , type_{ desc.type_ }
    , commandPool_{ VK_NULL_HANDLE }
{
    VkCommandPoolCreateInfo commandPoolInfo;
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.queueFamilyIndex = familyIndex_;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_ASSERT(table_->vkCreateCommandPool(device_->Handle(), &commandPoolInfo, nullptr, &commandPool_));

    for (auto i = 0u; i < desc.workersCount_; ++i) {
        WorkerDesc workerDesc;
        workerDesc.table_ = table_;
        workerDesc.device_ = device_;
        workerDesc.type_ = type_;
        workerDesc.queueFamilyIndex_ = familyIndex_;
        workerDesc.queueIndex_ = i;
        workerDesc.bufferingCount_ = 1;

        workers_.emplace_back(workerDesc);
    }
}

WorkerGroup::WorkerGroup(WorkerGroup&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , familyIndex_{ std::numeric_limits<std::uint32_t>::max() }
    , type_{ WorkerType::NONE }
    , commandPool_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

WorkerGroup& WorkerGroup::operator=(WorkerGroup&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(familyIndex_, rhs.familyIndex_);
    std::swap(type_, rhs.type_);
    std::swap(commandPool_, rhs.commandPool_);
    std::swap(workers_, rhs.workers_);

    return *this;
}

}