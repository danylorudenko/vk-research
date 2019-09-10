#pragma once

#include "..\..\class_features\NonCopyable.hpp"

#include "Worker.hpp"

namespace VAL
{

struct WorkerGroupDesc
{
    ImportTable* table_;
    Device* device_;

    WorkerType type_;
    std::uint32_t familyIndex_;
    std::uint32_t workersCount_;
    std::uint32_t bufferingCount_;
};

class WorkerGroup
    : public NonCopyable
{
public:
    WorkerGroup();
    WorkerGroup(WorkerGroupDesc const& desc);

    WorkerGroup(WorkerGroup&& rhs);
    WorkerGroup& operator=(WorkerGroup&& rhs);

    std::uint32_t WorkersCount() const;
    Worker* GetWorker(std::uint32_t index);

    void AllocCommandBuffers(std::uint32_t count, VkCommandBuffer* buffers);
    void FreeCommandBuffers(std::uint32_t count, VkCommandBuffer* buffers);

    ~WorkerGroup();

private:
    ImportTable* table_;
    Device* device_;

    std::uint32_t familyIndex_;
    WorkerType type_;

    VkCommandPool commandPool_;
    std::vector<Worker> workers_;
    

};

}