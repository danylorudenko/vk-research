#pragma once

#include <renderer\RendererDescriptorSet.hpp>
#include <renderer\RenderWorkItem.hpp>
#include <renderer\RootDef.hpp>
#include <vk_interface\VkInterfaceConstants.hpp>

namespace Render
{

struct RenderWorkItem;
class Root;

class UniformBufferWriterProxy
{
public:
    UniformBufferWriterProxy();
    UniformBufferWriterProxy(Root* root, UniformBufferHandle& item);
    UniformBufferWriterProxy(Root* root, RenderWorkItem* item, std::uint32_t setId, std::uint32_t setMemberId);
    UniformBufferWriterProxy(Root* root, MaterialKey const& materialKey, std::uint32_t passId, std::uint32_t setId, std::uint32_t setMemberId);

    UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs);
    UniformBufferWriterProxy& operator=(UniformBufferWriterProxy const& rhs);

    ~UniformBufferWriterProxy();

    bool IsMapped(std::uint32_t context) const;
    void* MappedPtr(std::uint32_t context) const;

    template<typename T>
    T* MappedPtr(std::uint32_t context) const { return reinterpret_cast<T*>(MappedPtr(context)); }

    void* MapForWrite(std::uint32_t context);
    void MapAllContexts();
    void Flush(std::uint32_t context) const;
    void FlushUnmap(std::uint32_t context);

private:
    Root* root_;
    void* mappedBufferPtr_[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    UniformBufferHandle uniformBufferHandle_;
};

}