#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;
class ResourcesController;

struct RenderPassAttachmentDesc
{
    VkFormat format_;
};

struct RenderPassDesc
{
    std::uint32_t colorAttachmentsCount_;
    RenderPassAttachmentDesc* colorAttachments_;
    RenderPassAttachmentDesc* depthStencilAttachment_;
};


struct RenderPassControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class RenderPassController
    : public NonCopyable
{
public:
    RenderPassController();
    RenderPassController(RenderPassControllerDesc const& desc);
    RenderPassController(RenderPassController&& rhs);
    RenderPassController& operator=(RenderPassController&& rhs);

    ~RenderPassController();


    RenderPassHandle AssembleRenderPass(RenderPassDesc const& desc);
    void FreeRenderPass(RenderPassHandle handle);

    RenderPass* GetRenderPass(RenderPassHandle handle);
    
private:
    ImportTable* table_;
    Device* device_;
    
    std::vector<RenderPass> renderPasses_;
};

}