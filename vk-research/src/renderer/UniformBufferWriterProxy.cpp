#include "UniformBufferWriterProxy.hpp"
#include "Root.hpp"
#include "..\vk_interface\ImportTable.hpp"

namespace Render
{
UniformBufferWriterProxy::UniformBufferWriterProxy()
    : root_{ nullptr }
    , mappedBufferPtr_{ nullptr }
    , uniformBufferHandle_{}
{
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, UniformBufferHandle& item)
    : root_{ root }
    , mappedBufferPtr_{ nullptr }
    , uniformBufferHandle_{ item }
{
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, RenderWorkItem* item, std::uint32_t setId, std::uint32_t setMemberId)
    : root_{ root }
    , mappedBufferPtr_{ nullptr }
    , uniformBufferHandle_{}
{
    uniformBufferHandle_ = item->descriptorSetsOwner_.slots_[setId].descriptorSet_.setMembers_[setMemberId].data_.uniformBuffer_.uniformBufferHandle_;
    
}

UniformBufferWriterProxy::UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs) = default;

UniformBufferWriterProxy& UniformBufferWriterProxy::operator=(UniformBufferWriterProxy const& rhs) = default;

UniformBufferWriterProxy::~UniformBufferWriterProxy() = default;


void* UniformBufferWriterProxy::MapForWrite(std::uint32_t context)
{
    mappedBufferPtr_[context] = root_->MapUniformBuffer(uniformBufferHandle_, context);
    return mappedBufferPtr_;
}

void UniformBufferWriterProxy::Flush(std::uint32_t context) const
{
    root_->FlushUniformBuffer(uniformBufferHandle_, context);
}

void UniformBufferWriterProxy::FlushUnmap(std::uint32_t context)
{
    root_->FlushUniformBuffer(uniformBufferHandle_, context);
    root_->UnmapUniformBuffer(uniformBufferHandle_, context);
    mappedBufferPtr_[context] = nullptr;

}

}