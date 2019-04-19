#pragma once

#include "RendererDescriptorSet.hpp"
#include "RenderWorkItem.hpp"
#include "RootDef.hpp"
#include "..\vk_interface\runtime\FramedDescriptorsHub.hpp"

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

    UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs);
    UniformBufferWriterProxy& operator=(UniformBufferWriterProxy const& rhs);

    ~UniformBufferWriterProxy();

    bool IsMapped(std::uint32_t context) const { return mappedBufferPtr_[context] != nullptr; }
    void* MappedPtr(std::uint32_t context) const { return mappedBufferPtr_[context]; }

    void* MapForWrite(std::uint32_t context);
    void Flush(std::uint32_t context) const;
    void FlushUnmap(std::uint32_t context);

private:
    Root* root_;
    void* mappedBufferPtr_[VKW::FramedDescriptorsHub::MAX_FRAMES_COUNT];
    UniformBufferHandle uniformBufferHandle_;
};

}