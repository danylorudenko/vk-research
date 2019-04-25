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
    // In the current implementation uniform buffer memory (aka HOST_VISIBLE memory)
    // is persistently-mapped, so the only thing we need is mapped memory ptr + offset for the resource.

    UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);

    std::uint64_t const offset = memoryRegion->offset_ + view->offset_;
    //std::uint64_t const size = view->size_;

    return (reinterpret_cast<std::uint8_t*>(memoryPage->mappedMemoryPtr_) + offset);


    //VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    //
    //VKW::ImportTable* table = loader_->table_.get();
    //VkDevice const vkDevice = loader_->device_->Handle();
    //
    //void* mappedRange = nullptr;
    //VK_ASSERT(table->vkMapMemory(vkDevice, deviceMemory, offset, size, VK_FLAGS_NONE, &mappedRange));
    //assert(mappedRange != nullptr && "Can't map uniform buffer.");
    //
    //return mappedRange;
}

void Root::UnmapUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    // In the current implementation uniform buffer's memory (aka HOST_VISIBLE memory)
    // is persistently mapped, so it can't be unmapped.

    //UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    //VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    //VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);
    //VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);
    //
    //VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    //
    //VKW::ImportTable* table = loader_->table_.get();
    //VkDevice const vkDevice = loader_->device_->Handle();
    //
    //table->vkUnmapMemory(vkDevice, deviceMemory);
}

void Root::FlushUniformBuffer(UniformBufferHandle handle, std::uint32_t frame)
{
    UniformBuffer uniformBuffer = uniformBuffers_[handle.id_];
    VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, frame);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);

    if(memoryPage->propertyFlags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        return;

    VKW::ImportTable* table = loader_->table_.get();
    VkDevice const vkDevice = loader_->device_->Handle();
    VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    std::uint64_t const offset = memoryRegion->offset_ + view->offset_;
    std::uint64_t const size = view->size_;

    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = deviceMemory;
    range.size = size;
    range.offset = offset;
    VK_ASSERT(table->vkFlushMappedMemoryRanges(vkDevice, 1, &range));
}

void* Root::MapBuffer(ResourceKey const& key, std::uint32_t frame)
{
    VKW::BufferView* view = FindGlobalBuffer(key, frame);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);

    std::uint64_t const offset = memoryRegion->offset_ + view->offset_;
    //std::uint64_t const size = view->size_;

    return (reinterpret_cast<std::uint8_t*>(memoryPage->mappedMemoryPtr_) + offset);
}

void Root::FlushBuffer(ResourceKey const& key, std::uint32_t frame)
{
    VKW::BufferView* view = FindGlobalBuffer(key, frame);
    VKW::MemoryPage const* memoryPage = GetViewMemoryPage(view);
    VKW::MemoryRegion const* memoryRegion = GetViewMemory(view);

    if (memoryPage->propertyFlags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        return;

    VKW::ImportTable* table = loader_->table_.get();
    VkDevice const vkDevice = loader_->device_->Handle();
    VkDeviceMemory const deviceMemory = memoryPage->deviceMemory_;
    std::uint64_t const offset = memoryRegion->offset_ + view->offset_;
    std::uint64_t const size = view->size_;

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
    passDesc.descriptorLayoutController_ = loader_->descriptorLayoutController_.get();
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

Shader& Root::FindShader(ShaderKey const& key)
{
    return shaderMap_[key];
}

void Root::DefineShader(ShaderKey const& key, ShaderDesc const& desc)
{
    Shader& shader = shaderMap_[key];

    VKW::ShaderModuleDesc moduleDesc;
    moduleDesc.shaderPath_ = desc.relativePath_.c_str();
    moduleDesc.type_ = desc.type_;

    shader.vkwShaderModuleHandle_ = shaderModuleFactory_->LoadModule(moduleDesc);
}

void Root::DefineGraphicsPipeline(PipelineKey const& key, GraphicsPipelineDesc const& desc)
{
    Pipeline& pipeline = pipelineMap_[key];
    pipeline.staticLayoutMembersCount_ = desc.layoutDesc_->staticMembersCount_;
    pipeline.instancedLayoutMembersCount_ = desc.layoutDesc_->instancedMembersCount_;
    
    VKW::PipelineLayoutDesc vkwLayoutDesc;
    vkwLayoutDesc.membersCount_ = desc.layoutDesc_->staticMembersCount_ + desc.layoutDesc_->instancedMembersCount_;

    std::uint32_t vkwMembersCounter = 0;

    std::uint32_t const staticMembersCount = desc.layoutDesc_->staticMembersCount_;
    for (std::uint32_t i = 0; i < staticMembersCount; ++i, ++vkwMembersCounter) {
        pipeline.staticLayoutKeys_[i] = desc.layoutDesc_->staticMembers_[i];
        SetLayout const& setLayout = FindSetLayout(desc.layoutDesc_->staticMembers_[i]);
        vkwLayoutDesc.members_[vkwMembersCounter] = setLayout.vkwSetLayoutHandle_;
    }

    std::uint32_t const instancedMembersCount = desc.layoutDesc_->instancedMembersCount_;
    for (std::uint32_t i = 0; i < instancedMembersCount; ++i, ++vkwMembersCounter) {
        pipeline.instancedLayoutKeys_[i] = desc.layoutDesc_->instancedMembers_[i];
        SetLayout const& setLayout = FindSetLayout(desc.layoutDesc_->instancedMembers_[i]);
        vkwLayoutDesc.members_[vkwMembersCounter] = setLayout.vkwSetLayoutHandle_;
    }
    
    VKW::GraphicsPipelineDesc vkwDesc;
    vkwDesc.optimized_ = desc.optimized_;
    vkwDesc.inputAssemblyInfo_ = desc.inputAssemblyInfo_;
    vkwDesc.vertexInputInfo_ = desc.vertexInputInfo_;
    vkwDesc.shaderStagesCount_ = desc.shaderStagesCount_;
    for (std::uint32_t i = 0u; i < desc.shaderStagesCount_; ++i) {
        Shader& shader = FindShader(desc.shaderStages_[i]);
        vkwDesc.shaderStages_[i] = VKW::ShaderStageInfo{ shader.vkwShaderModuleHandle_ };
    }
    vkwDesc.viewportInfo_ = desc.viewportInfo_;
    vkwDesc.layoutDesc_ = &vkwLayoutDesc;
    vkwDesc.renderPass_ = renderPassMap_[desc.renderPass_].VKWRenderPass();
    vkwDesc.depthStencilInfo_ = desc.depthStencilInfo_;
    
    VKW::PipelineHandle const vkwPipelineHandle = pipelineFactory_->CreateGraphicsPipeline(vkwDesc);
    VKW::Pipeline const* vkwPipeline = pipelineFactory_->GetPipeline(vkwPipelineHandle);
    VKW::PipelineLayoutHandle const vkwPipelineLayoutHandle = vkwPipeline->layoutHandle;


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

void Root::DecorateProxySetWriteDescription(VKW::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, UniformBufferHandle bufferHandle)
{
    UniformBuffer const& uniformBuffer = FindUniformBuffer(bufferHandle);

    std::uint32_t framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i) {
        auto& bufferInfo = writeDesc.frames_[i].bufferInfo_;
        
        VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, i);

        bufferInfo.pureBufferViewHandle_ = resourceProxy_->GetBufferViewHandle(uniformBuffer.proxyBufferViewHandle_, i);
        bufferInfo.offset_ = 0;
        bufferInfo.size_ = static_cast<std::uint32_t>(view->size_);
    }
}

void Root::InitializeSetsOwner(DescriptorSetsOwner& owner, std::uint32_t setsCount, SetLayoutKey const* setLayoutKeys, SetOwnerDesc const* setOwnerDescs)
{
    owner.slotsCount_ = setsCount;

    std::uint32_t const setLayoutsCount = setsCount;
    for (std::uint32_t i = 0; i < setLayoutsCount; ++i) {
        SetLayout const& setLayout = FindSetLayout(setLayoutKeys[i]);

        VKW::ProxySetHandle proxySet = resourceProxy_->CreateSet(setLayout.vkwSetLayoutHandle_);
        owner.slots_[i].descriptorSet_.setLayoutKey_ = setLayoutKeys[i];
        owner.slots_[i].descriptorSet_.proxyDescriptorSetHandle_ = proxySet;

        VKW::ProxyDescriptorWriteDesc descriptorsWriteDescs[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];

        std::uint32_t const setLayoutMembersCount = setLayout.membersCount_;
        for (std::uint32_t k = 0; k < setLayoutMembersCount; k++) {
            SetLayout::Member const& setLayoutMember = setLayout.membersInfo_[k];
            SetOwnerDesc::Member const& setMemberData = setOwnerDescs[i].members_[k];

            VKW::DescriptorType const memberType = setLayoutMember.type_;

            switch (memberType) {
            case VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                UniformBufferHandle uniformBufferHandle = AcquireUniformBuffer(setMemberData.uniformBuffer_.size_);
                DecorateProxySetWriteDescription(descriptorsWriteDescs[k], k, uniformBufferHandle);
                owner.slots_[i].descriptorSet_.setMembers_[k].data_.uniformBuffer_.uniformBufferHandle_ = uniformBufferHandle;
                break;
            }

        }

        resourceProxy_->WriteSet(proxySet, descriptorsWriteDescs);
    }
    
}

void Root::DefineMaterial(MaterialKey const& key, MaterialDesc const& desc)
{
    Material& material = materialMap_[key];
    MaterialTemplate const& materialTemplate = FindMaterialTemplate(desc.templateKey_);

    material.templateKey_ = desc.templateKey_;
    material.perPassDataCount_ = materialTemplate.perPassDataCount_;

    std::uint32_t const perPassDataCount = materialTemplate.perPassDataCount_;
    for (std::uint32_t i = 0; i < perPassDataCount; ++i) {
        MaterialTemplate::PerPassData const& templatePerPassData = materialTemplate.perPassData_[i];

        std::uint32_t const keysCount = templatePerPassData.materialLayoutKeysCount_;
        SetLayoutKey const* setLayoutKeys = templatePerPassData.materialLayoutKeys_;
        SetOwnerDesc const* setOwnerDescs = desc.perPassData_[i].setOwnerDesc_;

        InitializeSetsOwner(material.perPassData_[i].descritorSetsOwner_, keysCount, setLayoutKeys, setOwnerDescs);

        //std::uint32_t const setLayoutsCount = templatePerPassData.materialLayoutKeysCount_;
        //for (std::uint32_t j = 0; j < setLayoutsCount; ++j) {
        //    SetLayoutKey const& setLayoutKey = templatePerPassData.materialLayoutKeys_[j];
        //    SetLayout const& setLayout = FindSetLayout(setLayoutKey);
        //
        //    VKW::ProxySetHandle proxySet = resourceProxy_->CreateSet(setLayout.vkwSetLayoutHandle_);
        //    materialPerPassData.descritorSetSlots_.slots_[j].setHandle_ = proxySet;
        //
        //    VKW::ProxyDescriptorWriteDesc descriptorsWriteDescs[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
        //
        //    std::uint32_t const setLayoutMembersCount = setLayout.membersCount_;
        //    for (std::uint32_t k = 0; k < setLayoutMembersCount; k++) {
        //        SetLayout::Member const& setLayoutMember = setLayout.membersInfo_[k];
        //        SetOwnerDesc::Member const& setMemberData = desc.perPassData_[i].setData_[j].members_[k];
        //
        //        VKW::DescriptorType const memberType = setLayoutMember.type_;
        //
        //        switch (memberType) {
        //        case VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        //            UniformBufferHandle uniformBufferHandle = AcquireUniformBuffer(setMemberData.uniformBuffer_.size_);
        //            DecorateProxySetWriteDescription(descriptorsWriteDescs[k], k, uniformBufferHandle);
        //            break;
        //        }
        //        
        //    }
        //
        //    resourceProxy_->WriteSet(proxySet, descriptorsWriteDescs);
        //}
    }
}

Material& Root::FindMaterial(MaterialKey const& key)
{
    return materialMap_[key];
}

void Root::RegisterMaterial(MaterialKey const& key)
{
    Material& material = materialMap_[key];
    MaterialTemplate const& materialTemplate = materialTemplateMap_[material.templateKey_];
    std::uint32_t materialPassCount = material.perPassDataCount_;
    for (std::uint32_t i = 0; i < materialPassCount; ++i) {
        PipelineKey const& pipelineKey = materialTemplate.perPassData_[i].pipelineKey_;
        Material::PerPassData* perPassData = material.perPassData_ + i;

        Pass& pass = renderPassMap_[materialTemplate.perPassData_[i].passKey_];
        pass.RegisterMaterialData(perPassData, pipelineKey);

    }
}

RenderWorkItemHandle Root::ConstructRenderWorkItem(Pipeline& pipeline, RenderItemDesc const& desc)
{
    pipeline.renderItems_.emplace_back();
    RenderWorkItem& item = pipeline.renderItems_.back();

    item.vertexCount_ = desc.vertexCount_;
    
    InitializeSetsOwner(item.descriptorSetsOwner_, pipeline.instancedLayoutMembersCount_, pipeline.instancedLayoutKeys_, desc.setOwnerDescs_);
    

    return RenderWorkItemHandle{ static_cast<std::uint32_t>(pipeline.renderItems_.size() - 1) };
}

void Root::ReleaseRenderWorkItem(Pipeline& pipeline, RenderWorkItemHandle handle)
{
    //auto& item = pipeline.renderItems_[handle.id_];
    //auto const uniformBuffersCount = item.uniformBuffersCount_;
    //for (auto i = 0u; i < uniformBuffersCount; ++i) {
    //    auto& uniform = item.uniformBuffers_[i];
    //    ReleaseUniformBuffer(uniform.serverBufferHandle_);
    //    std::free(uniform.hostBuffer_);
    //}
}

RenderWorkItem* Root::FindRenderWorkItem(PipelineKey const& key, RenderWorkItemHandle handle)
{
    return &FindPipeline(key).renderItems_[handle.id_];
}

RenderWorkItem* Root::FindRenderWorkItem(Pipeline& pipeline, RenderWorkItemHandle handle)
{
    return &pipeline.renderItems_[handle.id_];
}

RenderWorkItemHandle Root::ConstructRenderWorkItem(PipelineKey const& pipelineKey, RenderItemDesc const& desc)
{
    return ConstructRenderWorkItem(FindPipeline(pipelineKey), desc);
}

void Root::ReleaseRenderWorkItem(PipelineKey const& pipelineKey, RenderWorkItemHandle handle)
{
    return ReleaseRenderWorkItem(FindPipeline(pipelineKey), handle);
}

void Root::CopyBuffer(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context)
{
    VKW::BufferView* srcView = FindGlobalBuffer(src, context);
    VKW::BufferView* dstView = FindGlobalBuffer(dst, context);
    
    VKW::BufferResource* srcBuffer = resourceProxy_->GetResource(srcView->providedBuffer_->bufferResource_);
    VKW::BufferResource* dstBuffer = resourceProxy_->GetResource(dstView->providedBuffer_->bufferResource_);
    
    VKW::WorkerFrameCommandReciever commandReciever = mainWorkerTemp_->StartExecutionFrame(context);

    VkBufferCopy copyInfo;
    copyInfo.srcOffset = srcView->offset_;
    copyInfo.dstOffset = dstView->offset_;
    copyInfo.size = srcView->size_;

    VulkanFuncTable()->vkCmdCopyBuffer(commandReciever.commandBuffer_, srcBuffer->handle_, dstBuffer->handle_, 1, &copyInfo);

    mainWorkerTemp_->EndExecutionFrame(context);

    mainWorkerTemp_->ExecuteFrame(context, VK_NULL_HANDLE);

    VulkanFuncTable()->vkDeviceWaitIdle(loader_->device_->Handle());
}

VKW::ResourceRendererProxy* Root::ResourceProxy() const
{
    return resourceProxy_;
}

void Root::PushPassTemp(RenderPassKey const& key)
{
    renderGraphRootTemp_.emplace_back(key);
}

VKW::PresentationContext Root::AcquireNextPresentationContext()
{
    return presentationController_->AcquireNewPresentationContext();
}

void Root::IterateRenderGraph(VKW::PresentationContext const& presentationContext)
{
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