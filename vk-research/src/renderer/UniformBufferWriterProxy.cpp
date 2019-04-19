#include "UniformBufferWriterProxy.hpp"
#include "Root.hpp"
#include "..\vk_interface\ImportTable.hpp"

namespace Render
{
UniformBufferWriterProxy::UniformBufferWriterProxy()
    : root_{ nullptr }
    , uniformBufferHandle_{}
{
}

UniformBufferWriterProxy::UniformBufferWriterProxy(Root* root, UniformBufferHandle& item)
    : root_{ root }
    , uniformBufferHandle_{ item }
{
}

UniformBufferWriterProxy::UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs) = default;

UniformBufferWriterProxy& UniformBufferWriterProxy::operator=(UniformBufferWriterProxy const& rhs) = default;


void* UniformBufferWriterProxy::MapForWrite(/* std::uint32_t context */)
{
    UniformBuffer& uniformBuffer = root_->FindUniformBuffer(uniformBufferHandle_);
    return root_->ResourceProxy()->MapBuffer(uniformBuffer.proxyBufferViewHandle_);;
}

}