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

void* Root::MapUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);

    VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    // fuck, buffer view contains no size for buffers with undefined format
    auto const size = view->size_;
    auto const offset = memoryRegion->offset_;

    VKW::ImportTable* table = loader_->table_.get();
    VkDevice const vkDevice = loader_->device_->Handle();

    void* mappedRange = nullptr;
    VK_ASSERT(table->vkMapMemory(vkDevice, deviceMemory, offset, size, VK_FLAGS_NONE, &mappedRange));
    assert(mappedRange != nullptr && "Can't map uniform buffer.");

    return mappedRange;
}

void Root::UnmapUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);

    VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;

    VKW::ImportTable* table = loader_->table_.get();
    VkDevice const vkDevice = loader_->device_->Handle();

    table->vkUnmapMemory(vkDevice, deviceMemory);
}

void Root::FlushUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);

    VKW::ImportTable* table = loader_->table_.get();
    VkDevice const vkDevice = loader_->device_->Handle();
    VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    auto const offset = memoryRegion->offset_;
    auto const size = view->size_;

    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = deviceMemory;
    range.size = size;
    range.offset = offset;
    VK_ASSERT(table->vkFlushMappedMemoryRanges(vkDevice, 1, &range));
}

VKW::BufferResource* Root::GetViewResource(VKW::BufferView* view)
{
    VKW::BufferResourceHandle resourceHandle = view->providedBuffer_->bufferResource_;
    return resourceProxy_->GetResource(resourceHandle);
}

VKW::MemoryRegion* Root::GetViewMemory(VKW::BufferView* view)
{
    VKW::BufferResourceHandle resourceHandle = view->providedBuffer_->bufferResource_;
    VKW::BufferResource* bufferResource = resourceProxy_->GetResource(resourceHandle);
    return &bufferResource->memory_;
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
    SetLayout& set = setLayoutMap_[key];
    set.vkwSetLayoutHandle_ = handle;
    set.membersCount_ = desc.membersCount_;
    
    VKW::DescriptorSetLayout* layout = layoutController_->GetDescriptorSetLayout(handle);
    std::uint32_t const membersCount = layout->membersCount_;
    for (std::uint32_t i = 0; i < membersCount; ++i) {
        set.membersInfo_[i].type_ = desc.membersDesc_[i].type_;
        set.membersInfo_[i].binding_ = desc.membersDesc_[i].binding_;
    }
}

SetLayout& Root::FindSetLayout(SetLayoutKey const& key)
{
    return setLayoutMap_[key];
}

void Root::DefineGraphicsPipeline(PipelineKey const& key, GraphicsPipelineDesc const& desc)
{
    VKW::PipelineLayoutDesc vkwLayoutDesc;
    vkwLayoutDesc.membersCount_ = desc.layoutDesc_->staticMembersCount_ + desc.layoutDesc_->instancedMembersCount_;

    std::uint32_t vkwMembersCounter = 0;

    std::uint32_t const staticMembersCount = desc.layoutDesc_->staticMembersCount_;
    for (std::uint32_t i = 0; i < staticMembersCount; ++i, ++vkwMembersCounter) {
        SetLayout const& setLayout = FindSetLayout(desc.layoutDesc_->staticMembers_[i]);
        vkwLayoutDesc.members_[vkwMembersCounter] = setLayout.vkwSetLayoutHandle_;
    }

    std::uint32_t const instancedMembersCount = desc.layoutDesc_->instancedMembersCount_;
    for (std::uint32_t i = 0; i < instancedMembersCount; ++i, ++vkwMembersCounter) {
        SetLayout const& setLayout = FindSetLayout(desc.layoutDesc_->instancedsMembers_[i]);
        vkwLayoutDesc.members_[vkwMembersCounter] = setLayout.vkwSetLayoutHandle_;
    }
    
    VKW::GraphicsPipelineDesc vkwDesc;
    vkwDesc.optimized_ = desc.optimized_;
    vkwDesc.inputAssemblyInfo_ = desc.inputAssemblyInfo_;
    vkwDesc.vertexInputInfo_ = desc.vertexInputInfo_;
    vkwDesc.shaderStagesCount_ = desc.shaderStagesCount_;
    for (std::uint32_t i = 0u; i < desc.shaderStagesCount_; ++i) {
        Shader& shader = FindShader(desc.shaderStages_[i].shaderKey_);
        vkwDesc.shaderStages_[i] = VKW::ShaderStageInfo{ shader.vkwShaderModuleHandle_ };
    }
    vkwDesc.viewportInfo_ = desc.viewportInfo_;
    vkwDesc.layoutDesc_ = &vkwLayoutDesc;
    vkwDesc.renderPass_ = renderPassMap_[desc.renderPass_].VKWRenderPass();
    vkwDesc.depthStencilInfo_ = desc.depthStencilInfo_;
    
    VKW::PipelineHandle const vkwPipelineHandle = pipelineFactory_->CreateGraphicsPipeline(vkwDesc);
    VKW::Pipeline const* vkwPipeline = pipelineFactory_->GetPipeline(vkwPipelineHandle);
    VKW::PipelineLayoutHandle const vkwPipelineLayoutHandle = vkwPipeline->layoutHandle;

    Pipeline& pipeline = pipelineMap_[key];
    pipeline.staticLayoutMembersCount_ = desc.layoutDesc_->staticMembersCount_;
    pipeline.instancedLayoutMembersCount_ = desc.layoutDesc_->instancedMembersCount_;
    pipeline.layoutHandle_ = vkwPipelineLayoutHandle;
    pipeline.pipelineHandle_ = vkwPipelineHandle;
}

Pipeline& Root::FindPipeline(PipelineKey const& key)
{
    return pipelineMap_[key];
}

void Root::DefineMaterialTemplate(MaterialTemplateKey const& key, MaterialTemplateDesc const& desc)
{
    // instantiate the struct
    MaterialTemplate& materialTemplate = materialTemplateMap_[key];
    materialTemplate.perPassDataCount_ = desc.perPassDataCount_;

    std::uint32_t perPassDataCount = desc.perPassDataCount_;
    for (std::uint32_t i = 0; i < perPassDataCount; ++i) {
        MaterialTemplateDesc::PerPassData const& perPassDataDesc = desc.perPassData_[i];
        MaterialTemplate::PerPassData& perPassData = materialTemplate.perPassData_[i];

        perPassData.passKey_ = perPassDataDesc.passKey_;
        
        Pipeline& pipeline = FindPipeline(perPassDataDesc.pipelineKey_);
        VKW::PipelineLayout* vkwPipelineLayout = layoutController_->GetPipelineLayout(pipeline.layoutHandle_);
        
        perPassData.pipelineKey_ = perPassDataDesc.pipelineKey_;

        perPassData.materialLayoutKeysCount_ = pipeline.staticLayoutMembersCount_;
        perPassData.renderitemLayoutKeysCount_ = pipeline.instancedLayoutMembersCount_;

        std::uint32_t generalLayoutsCounter = 0;

        std::uint32_t const materialKeysCount = pipeline.staticLayoutMembersCount_;
        for (std::uint32_t j = 0; j < materialKeysCount; ++j, ++generalLayoutsCounter) {
            perPassData.materialLayoutKeys_[j] = pipeline.staticLayoutKeys_[generalLayoutsCounter];
        }

        std::uint32_t const instanceKeysCount = pipeline.instancedLayoutMembersCount_;
        for (std::uint32_t j = 0; j < instanceKeysCount; ++j, ++generalLayoutsCounter) {
            perPassData.renderitemLayoutKeys_[j] = pipeline.instancedLayoutKeys_[generalLayoutsCounter];
        }
    }
}

MaterialTemplate& Root::FindMaterialTemplate(MaterialTemplateKey const& key)
{
    return materialTemplateMap_[key];
}

void Root::DefineMaterial(MaterialKey const& key, MaterialDesc const& desc)
{
    Material& material = materialMap_[key];

    MaterialTemplate const& materialTemplate = FindMaterialTemplate(desc.templateKey_);

    std::uint32_t const perPassDataCount = materialTemplate.perPassDataCount_;
    for (std::uint32_t i = 0; perPassDataCount; ++i) {
        MaterialTemplate::PerPassData const& perPassData = materialTemplate.perPassData_[i];

        std::uint32_t const setLayoutsCount = perPassData.materialLayoutKeysCount_;
        for (std::uint32_t j = 0; j < setLayoutsCount; ++j) {
            SetLayoutKey const& setLayoutKey = perPassData.materialLayoutKeys_[j];
            SetLayout const& setLayout = FindSetLayout(setLayoutKey);

            std::uint32_t const setLayoutMembersCount = setLayout.membersCount_;
            for (std::uint32_t k = 0; k < setLayoutMembersCount; k++) {
                SetLayout::Member const& setLayoutMember = setLayout.membersInfo_[k];
                MaterialDesc::PerPassData::SetData::Member const& setLayoutMemberData = desc.perPassData_[i].setData_[j].members_[k];

                VKW::DescriptorType const memberType = setLayoutMember.type_;
                switch (memberType) {
                case VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    // hmmmmmm, should we create descriptor set for all frames in place?
                    // or should we wrap it somehow in a "DescriptorSetOwner" ow some kind?
                    //this->Set
                    break;
                }
            }
        }
    }
}

Material& Root::FindMaterial(MaterialKey const& key)
{
    return materialMap_[key];
}

RenderItemHandle Root::ConstructRenderItem(Pipeline& pipeline, RenderItemDesc const& desc)
{
    pipeline.renderItems_.emplace_back();
    auto& item = pipeline.renderItems_.back();

    item.vertexCount_ = desc.vertexCount_;
    
    item.descriptorSetCount_ = desc.setCount_;
    auto const setCount = desc.setCount_;
    for (auto i = 0u; i < setCount; ++i) {
        
        auto& setDesc = desc.requiredSetsDescs_[i];
        auto& set = item.descriptorSets_[i];

        SetLayout& setLayout = FindSetLayout(setDesc.setLayout_);
        VKW::DescriptorSetLayout* vkwSetLayout = layoutController_->GetDescriptorSetLayout(setLayout.vkwSetLayoutHandle_);
        VKW::ProxySetHandle proxyDescriptorSetHandle = resourceProxy_->CreateSet(setLayout.vkwSetLayoutHandle_);
        VKW::ProxyDescriptorDesc descriptorContentDescriptions[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
        
        set.setLayoutKey_ = setDesc.setLayout_;
        set.proxyDescriptorSetHandle_ = proxyDescriptorSetHandle;

        auto const setMembersCount = vkwSetLayout->membersCount_;
        for (auto j = 0u; j < setMembersCount; ++j) {
            auto& setMember = set.setMembers_[j];
            auto& descriptorContentDesc = descriptorContentDescriptions[j];

            // TODO
            setMember.name_[0] = '\0';

            switch (vkwSetLayout->membersInfo_[j].type_) {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                auto const& uniformBufferSetMemberData = setDesc.setMemberData_[j].uniformBufferSetMemberData_;
                auto const uniformBufferSize = uniformBufferSetMemberData.size_;

                UniformBufferSetMember& uniformBufferSetMember = setMember.data_.uniformBuffer_;
                uniformBufferSetMember.hostBufferSize_ = uniformBufferSize;
                uniformBufferSetMember.hostBuffer_ = reinterpret_cast<std::uint8_t*>(malloc(uniformBufferSize));
                uniformBufferSetMember.uniformBufferHandle_ = AcquireUniformBuffer(uniformBufferSize);

                auto const framesCount = resourceProxy_->FramesCount();
                for (auto frameNum = 0u; frameNum < framesCount; ++frameNum) {
                    auto& bufferDescriptorDesc = descriptorContentDesc.frames_[frameNum].bufferInfo_;

                    Render::UniformBuffer uniformBuffer = FindUniformBuffer(uniformBufferSetMember.uniformBufferHandle_);
                    VKW::BufferViewHandle uniformBufferViewHandle = resourceProxy_->GetBufferViewHandle(uniformBuffer.proxyBufferViewHandle_, frameNum);
                    VKW::BufferView* uniformBufferView = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frameNum);
                    bufferDescriptorDesc.pureBufferViewHandle_ = uniformBufferViewHandle;
                    bufferDescriptorDesc.size_ = static_cast<std::uint32_t>(uniformBufferView->size_);
                    bufferDescriptorDesc.offset_ = static_cast<std::uint32_t>(uniformBufferView->offset_);
                }
                break;
            }
        }

        // Updating the set content
        resourceProxy_->WriteSet(proxyDescriptorSetHandle, descriptorContentDescriptions);
    }

    return RenderItemHandle{ static_cast<std::uint32_t>(pipeline.renderItems_.size() - 1) };
}

void Root::ReleaseRenderItem(Pipeline& pipeline, RenderItemHandle handle)
{
    //auto& item = pipeline.renderItems_[handle.id_];
    //auto const uniformBuffersCount = item.uniformBuffersCount_;
    //for (auto i = 0u; i < uniformBuffersCount; ++i) {
    //    auto& uniform = item.uniformBuffers_[i];
    //    ReleaseUniformBuffer(uniform.serverBufferHandle_);
    //    std::free(uniform.hostBuffer_);
    //}
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