#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "RenderPass.hpp"
#include "../resources/Resource.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;
class ResourcesController;

struct RenderPassDesc
{
    std::uint32_t colorAttachmentsCount_;
    ImageResourceHandle* colorAttachments_;
    ImageResourceHandle depthStencilAttachment_;
};


struct RenderPassFactoryDesc
{
    ImportTable* table_;
    Device* device_;
    
    ResourcesController* resourcesController_;
};

class RenderPassFactory
    : public NonCopyable
{
public:
    RenderPassFactory();
    RenderPassFactory(RenderPassFactoryDesc const& desc);
    RenderPassFactory(RenderPassFactory&& rhs);
    RenderPassFactory& operator=(RenderPassFactory&& rhs);

    ~RenderPassFactory();


    RenderPassHandle AssembleRenderPass(RenderPassDesc const& desc);
    void FreeRenderPass(RenderPassHandle handle);
    
private:
    ImportTable* table_;
    Device* device_;

    ResourcesController* resourcesController_;

    std::vector<RenderPass> renderPasses_;
};

}