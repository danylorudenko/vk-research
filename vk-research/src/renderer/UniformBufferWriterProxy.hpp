#pragma once

#include "RendererDescriptorSet.hpp"
#include "RenderWorkItem.hpp"
#include "RootDef.hpp"

namespace Render
{

struct RenderWorkItem;
class Root;

class UniformBufferWriterProxy
{
public:
    UniformBufferWriterProxy();
    UniformBufferWriterProxy(Root* root, UniformBufferHandle& item);

    UniformBufferWriterProxy(UniformBufferWriterProxy const& rhs);
    UniformBufferWriterProxy& operator=(UniformBufferWriterProxy const& rhs);

    bool IsMapped();
    void* MapForWrite();
    void UnmapFlush();

private:
    Root* root_;
    UniformBufferHandle uniformBufferHandle_;
};

}