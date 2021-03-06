#include "UniformBufferWriterProxy.hpp"
#include "Root.hpp"
#include "..\vk_interface\ImportTable.hpp"

namespace Render
{

UniformBufferWriterProxy::UniformBufferWriterProxy()
    : root_{ nullptr }
    , mappedBufferPtr_{ nullptr, nullptr, nullptr }
    , uniformBufferHandle_{}
{
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, UniformBufferHandle& item)
    : root_{ root }
    , mappedBufferPtr_{ nullptr, nullptr, nullptr }
    , uniformBufferHandle_{ item }
{
    MapAllContexts();
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, RenderWorkItem* item, std::uint32_t setId, std::uint32_t setMemberId)
    : root_{ root }
    , mappedBufferPtr_{ nullptr, nullptr, nullptr }
    , uniformBufferHandle_{}
{
    uniformBufferHandle_ = item->descriptorSetsOwner_.slots_[setId].descriptorSet_.setMembers_[setMemberId].data_.uniformBuffer_.uniformBufferHandle_;
    MapAllContexts();
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, MaterialKey const& materialKey, std::uint32_t passId, std::uint32_t setId, std::uint32_t setMemberId)
    : root_{ root }
    , mappedBufferPtr_{ nullptr, nullptr, nullptr }
    , uniformBufferHandle_{}
{
    Material const& material = root_->FindMaterial(materialKey);
    uniformBufferHandle_ = material.perPassData_[passId].descritorSetsOwner_.slots_[setId].descriptorSet_.setMembers_[setMemberId].data_.uniformBuffer_.uniformBufferHandle_;
    MapAllContexts();
}

UniformBufferWriterProxy::UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs) = default;

UniformBufferWriterProxy& UniformBufferWriterProxy::operator=(UniformBufferWriterProxy const& rhs) = default;

UniformBufferWriterProxy::~UniformBufferWriterProxy() = default;


bool UniformBufferWriterProxy::IsMapped(std::uint32_t context) const
{
    return mappedBufferPtr_[context] != nullptr;
}

void* UniformBufferWriterProxy::MappedPtr(std::uint32_t context) const
{
    return mappedBufferPtr_[context];
}

void* UniformBufferWriterProxy::MapForWrite(std::uint32_t context)
{
    mappedBufferPtr_[context] = root_->MapUniformBuffer(uniformBufferHandle_, context);
    return mappedBufferPtr_[context];
}

void UniformBufferWriterProxy::MapAllContexts()
{
    std::uint32_t const framesCount = root_->ResourceProxy()->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i) {
        mappedBufferPtr_[i] = root_->MapUniformBuffer(uniformBufferHandle_, i);
    }
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