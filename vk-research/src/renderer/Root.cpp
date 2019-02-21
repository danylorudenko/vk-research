#include <utility>
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "..\vk_interface\worker\Worker.hpp"
#include "..\vk_interface\Loader.hpp"

namespace Render
{

Root::Root()
    : loader_{ nullptr }
    , resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , imagesProvider_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
    , layoutController_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , pipelineFactory_{ nullptr }
    , presentationController_{ nullptr }
    , mainWorkerTemp_{ nullptr }
    , defaultFramebufferWidth_{ 0 }
    , defaultFramebufferHeight_{ 0 }
    , nextUniformBufferId_{ 0u }
{

}

Root::Root(RootDesc const& desc)
    : loader_{ desc.loader_ }
    , resourceProxy_{ desc.resourceProxy_ }
    , renderPassController_{ desc.renderPassController_ }
    , imagesProvider_{ desc.imagesProvider_ }
    , framedDescriptorsHub_{ desc.framedDescriptorsHub_ }
    , layoutController_{ desc.layoutController_ }
    , shaderModuleFactory_{ desc.shaderModuleFactory_ }
    , pipelineFactory_{ desc.pipelineFactory_ }
    , presentationController_{ desc.presentationController_ }
    , mainWorkerTemp_{ desc.mainWorkerTemp_ }
    , defaultFramebufferWidth_{ desc.defaultFramebufferWidth_ }
    , defaultFramebufferHeight_{ desc.defaultFramebufferHeight_ }
    , nextUniformBufferId_{ 0u }
{
    VKW::ProxyImageHandle swapchainView = resourceProxy_->RegisterSwapchainImageViews();
    globalImages_[SWAPCHAIN_IMAGE_KEY] = swapchainView;
}

Root::Root(Root&& rhs)
    : loader_{ nullptr }
    , resourceProxy_{ nullptr }
    , renderPassController_{ nullptr }
    , imagesProvider_{ nullptr }
    , framedDescriptorsHub_{ nullptr }
    , layoutController_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , pipelineFactory_{ nullptr }
    , presentationController_{ nullptr }
    , mainWorkerTemp_{ nullptr }
    , defaultFramebufferWidth_{ 0 }
    , defaultFramebufferHeight_{ 0 }
    , nextUniformBufferId_{ 0u }
{
    operator=(std::move(rhs));
}

Root& Root::operator=(Root&& rhs)
{
    std::swap(loader_, rhs.loader_);
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
    std::swap(framedDescriptorsHub_, rhs.framedDescriptorsHub_);
    std::swap(layoutController_, rhs.layoutController_);
    std::swap(shaderModuleFactory_, rhs.shaderModuleFactory_);
    std::swap(pipelineFactory_, rhs.pipelineFactory_);
    std::swap(presentationController_, rhs.presentationController_);
    std::swap(mainWorkerTemp_, rhs.mainWorkerTemp_);
    std::swap(defaultFramebufferWidth_, rhs.defaultFramebufferWidth_);
    std::swap(defaultFramebufferHeight_, rhs.defaultFramebufferHeight_);
    std::swap(globalBuffers_, rhs.globalBuffers_);
    std::swap(globalImages_, rhs.globalImages_);
    std::swap(uniformBuffers_, rhs.uniformBuffers_);
    std::swap(nextUniformBufferId_, rhs.nextUniformBufferId_);
    std::swap(renderPassMap_, rhs.renderPassMap_);
    std::swap(setLayoutMap_, rhs.setLayoutMap_);
    std::swap(pipelineMap_, rhs.pipelineMap_);
    std::swap(renderGraphRootTemp_, rhs.renderGraphRootTemp_);

    return *this;
}

Root::~Root()
{

}

VKW::ImportTable* Root::VulkanFuncTable() const
{
    return loader_->table_.get();
}

UniformBufferHandle Root::AcquireUniformBuffer(std::uint32_t size)
{
    VKW::BufferViewDesc desc;
    desc.usage_ = VKW::BufferUsage::UNIFORM;
    desc.size_ = size;
    desc.format_ = VK_FORMAT_UNDEFINED;
    
    VKW::ProxyBufferHandle bufferHandle = resourceProxy_->CreateBuffer(desc);
    UniformBufferHandle handle { nextUniformBufferId_++ };
    uniformBuffers_[handle.id_] = UniformBuffer{ bufferHandle };

    return handle;
}

UniformBuffer& Root::FindUniformBuffer(UniformBufferHandle handle)
{
    return uniformBuffers_[handle.id_];
}

VKW::BufferView* Root::FindUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    auto const& proxyHandle = uniformBuffers_[handle.id_].proxyBufferViewHandle_;
    return resourceProxy_->GetBufferView(proxyHandle, frame);
}

void Root::ReleaseUniformBuffer(UniformBufferHandle handle)
{
    // no implementation
    std::cout << "Attampt to release uniform buffer by id: " << handle.id_ << std::endl;
}

VKW::BufferResource* Root::GetViewResource(VKW::BufferView* view)
{
    VKW::BufferResourceHandle resourceHandle = view->providedBuffer_->bufferResource_;
    return resourceProxy_->GetResource(resourceHandle);
}

VKW::MemoryPage* Root::GetViewMemoryPage(VKW::BufferView* view)
{
    VKW::BufferResourceHandle resourceHandle = view->providedBuffer_->bufferResource_;
    return resourceProxy_->GetMemoryPage(resourceHandle);
}

void Root::DefineGlobalBuffer(ResourceKey const& key, VKW::BufferViewDesc const& desc)
{
    VKW::ProxyBufferHandle bufferHandle = resourceProxy_->CreateBuffer(desc);
    globalBuffers_[key] = bufferHandle;
}

VKW::ProxyBufferHandle Root::FindGlobalBuffer(ResourceKey const& key)
{
    return globalBuffers_[key];
}

VKW::BufferView* Root::FindGlobalBuffer(ResourceKey const& key, std::uint32_t frame)
{
    auto const& proxyHandle = globalBuffers_[key];
    return resourceProxy_->GetBufferView(proxyHandle, frame);
}

void Root::DefineGlobalImage(ResourceKey const& key, VKW::ImageViewDesc const& desc)
{
    VKW::ProxyImageHandle imageHandle = resourceProxy_->CreateImage(desc);
    globalImages_[key] = imageHandle;
}

VKW::ProxyImageHandle Root::FindGlobalImage(ResourceKey const& key)
{
    return globalImages_[key];
}

VKW::ImageView* Root::FindGlobalImage(ResourceKey const& key, std::uint32_t frame)
{
    auto const& proxyHandle = globalImages_[key];
    return resourceProxy_->GetImageView(proxyHandle, frame);
}

void Root::DefineRenderPass(RenderPassKey const& key, RenderPassDesc const& desc)
{
    PassDesc passDesc;
    passDesc.root_ = this;
    passDesc.table_ = loader_->table_.get();
    passDesc.device_ = loader_->device_.get();
    passDesc.proxy_ = resourceProxy_;
    passDesc.renderPassController_ = renderPassController_;
    passDesc.pipelineFactory_ = pipelineFactory_;
    passDesc.framedDescriptorsHub_ = framedDescriptorsHub_;
    passDesc.imagesProvider_ = imagesProvider_;
    passDesc.width_ = defaultFramebufferWidth_;
    passDesc.height_ = defaultFramebufferHeight_;
    passDesc.colorAttachmentCount_ = desc.colorAttachmentsCount_;
    for (auto i = 0u; i < passDesc.colorAttachmentCount_; ++i) {
        passDesc.colorAttachments_[i] = globalImages_[desc.colorAttachments_[i]];
    }
    passDesc.depthStencilAttachment_ = desc.depthStencilAttachment_.size() > 0 ? &globalImages_[desc.depthStencilAttachment_] : nullptr;

    renderPassMap_[key] = Pass{ passDesc };
}

Pass& Root::FindPass(RenderPassKey const& key)
{
    return renderPassMap_[key];
}

void Root::DefineSetLayout(SetLayoutKey const& key, VKW::DescriptorSetLayoutDesc const& desc)
{
    VKW::DescriptorSetLayoutHandle handle = layoutController_->CreateDescriptorSetLayout(desc);
    setLayoutMap_[key] = SetLayout{ handle };
}

SetLayout& Root::FindSetLayout(SetLayoutKey const& key)
{
    return setLayoutMap_[key];
}

void Root::DefineGraphicsPipeline(PipelineKey const& key, GraphicsPipelineDesc const& desc)
{
    VKW::GraphicsPipelineDesc vkwDesc;
    vkwDesc.optimized_ = desc.optimized_;
    vkwDesc.inputAssemblyInfo_ = desc.inputAssemblyInfo_;
    vkwDesc.vertexInputInfo_ = desc.vertexInputInfo_;
    vkwDesc.shaderStagesCount_ = desc.shaderStagesCount_;
    for (auto i = 0u; i < desc.shaderStagesCount_; ++i) {
        VKW::ShaderModuleHandle moduleHandle = shaderModuleFactory_->LoadModule(desc.shaderStages_[i].desc_);
        vkwDesc.shaderStages_[i] = VKW::ShaderStageInfo{ moduleHandle };
    }
    vkwDesc.viewportInfo_ = desc.viewportInfo_;
    vkwDesc.layoutDesc_ = desc.layoutDesc_;
    vkwDesc.renderPass_ = renderPassMap_[desc.renderPass_].VKWRenderPass();
    vkwDesc.depthStencilInfo_ = desc.depthStencilInfo_;
    
    VKW::PipelineHandle handle = pipelineFactory_->CreateGraphicsPipeline(vkwDesc);
    // TODO: construction not complete
    pipelineMap_[key] = Pipeline{ handle };
}

Pipeline& Root::FindPipeline(PipelineKey const& key)
{
    return pipelineMap_[key];
}

RenderItemHandle Root::ConstructRenderItem(Pipeline& pipeline, RenderItemDesc const& desc)
{
    pipeline.renderItems_.emplace_back();
    auto& item = pipeline.renderItems_.back();

    item.vertexCount_ = desc.vertexCount_;
    
    item.uniformBuffersCount_ = desc.uniformBuffersCount_;
    auto const uniformBuffersCount = desc.uniformBuffersCount_;
    for (auto i = 0u; i < uniformBuffersCount; ++i) {
        
        auto& uniformBuffDesc = desc.uniformBuffersDescs[i];
        auto& uniform = item.uniformBuffers_[i];

        std::strcpy(uniform.name_, uniformBuffDesc.name_);

        auto const dataSize = uniformBuffDesc.size_;
        uniform.hostBuffer_ = reinterpret_cast<std::uint8_t*>(malloc(dataSize));
        uniform.hostBufferSize_ = dataSize;
        
        uniform.serverBufferHandle_ = AcquireUniformBuffer(dataSize);
    }

    return RenderItemHandle{ static_cast<std::uint32_t>(pipeline.renderItems_.size() - 1) };
}

void Root::ReleaseRenderItem(Pipeline& pipeline, RenderItemHandle handle)
{
    auto& item = pipeline.renderItems_[handle.id_];
    auto const uniformBuffersCount = item.uniformBuffersCount_;
    for (auto i = 0u; i < uniformBuffersCount; ++i) {
        auto& uniform = item.uniformBuffers_[i];
        ReleaseUniformBuffer(uniform.serverBufferHandle_);
        std::free(uniform.hostBuffer_);
    }
}

RenderItem* Root::FindRenderItem(Pipeline& pipeline, RenderItemHandle handle)
{
    return &pipeline.renderItems_[handle.id_];
}

RenderItemHandle Root::ConstructRenderItem(PipelineKey const& pipelineKey, RenderItemDesc const& desc)
{
    return ConstructRenderItem(FindPipeline(pipelineKey), desc);
}

void Root::ReleaseRenderItem(PipelineKey const& pipelineKey, RenderItemHandle handle)
{
    return ReleaseRenderItem(FindPipeline(pipelineKey), handle);
}

VKW::ResourceRendererProxy* Root::ResourceProxy() const
{
    return resourceProxy_;
}

void Root::PushPassTemp(RenderPassKey const& key)
{
    renderGraphRootTemp_.emplace_back(key);
}

void Root::IterateRenderGraph()
{
    VKW::PresentationContext const presentationContext = presentationController_->AcquireNewPresentationContext();
    std::uint32_t const contextId = presentationContext.contextId_;

    auto commandReciever = mainWorkerTemp_->StartExecutionFrame(contextId);

    std::uint32_t const passCount = static_cast<std::uint32_t>(renderGraphRootTemp_.size());
    for (auto i = 0u; i < passCount; ++i) {
        auto& pass = renderPassMap_[renderGraphRootTemp_[i]];
        pass.Begin(contextId, &commandReciever);
        pass.Render(contextId, &commandReciever);
        pass.End(contextId, &commandReciever);
    }

    mainWorkerTemp_->EndExecutionFrame(contextId);
    auto renderingCompleteSemaphore = mainWorkerTemp_->ExecuteFrame(contextId, presentationContext.contextPresentationCompleteSemaphore_);

    presentationController_->PresentContextId(contextId, renderingCompleteSemaphore);
}

}