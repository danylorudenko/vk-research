#include <utility>
#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"
#include "..\vk_interface\worker\Worker.hpp"
#include "..\vk_interface\Loader.hpp"
#include "..\vk_interface\Swapchain.hpp"
#include "..\vk_interface\VkInterfaceConstants.hpp"
#include "CustomTempBlurPass.hpp"

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
    , nextUniformBufferId_{ 0u }
{
    VKW::ProxyImageHandle swapchainView = resourceProxy_->RegisterSwapchainImageViews();
    globalImages_[SWAPCHAIN_IMAGE_KEY] = swapchainView;


    VKW::ImageViewDesc finalColorDesc;
    finalColorDesc.format_ = loader_->swapchain_->Format();
    finalColorDesc.width_ = loader_->swapchain_->Width() + COLOR_BUFFER_THREESHOLD * 2;
    finalColorDesc.height_ = loader_->swapchain_->Height() + COLOR_BUFFER_THREESHOLD * 2;
    finalColorDesc.usage_ = VKW::ImageUsage::RENDER_TARGET;

    DefineGlobalImage(GetDefaultSceneColorOutput(), finalColorDesc);

    VkImage swapchainImages[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    VkImageLayout imageLayouts[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];

    std::uint32_t const swapchainImagesCount = loader_->swapchain_->ImageCount();
    for (std::uint32_t i = 0; i < swapchainImagesCount; ++i) 
    {
        swapchainImages[i] = loader_->swapchain_->Image(i).image_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    ImageLayoutTransition(0, swapchainImagesCount, swapchainImages, imageLayouts);
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
    range.size = VK_WHOLE_SIZE;
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
    range.size = VK_WHOLE_SIZE;
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

ResourceKey Root::GetDefaultSceneColorOutput() const
{
    return SCENE_COLOR_OUTPUT_KEY;
}

ResourceKey Root::GetSwapchain() const
{
    return SWAPCHAIN_IMAGE_KEY;
}

std::uint32_t Root::GetDefaultSceneColorBufferThreeshold() const
{
    return COLOR_BUFFER_THREESHOLD;
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

void Root::DefineCustomBlurPass(PassKey const& key, ResourceKey const& sceneColorBuffer)
{
    //Root* root_;
    //
    //VKW::ImportTable* table_;
    //VKW::Device* device_;
    //
    //VKW::ResourceRendererProxy* resourceProxy_;
    //VKW::ShaderModuleFactory* shaderModuleFactory_;
    //VKW::PipelineFactory* pipelineFactory_;
    //VKW::DescriptorLayoutController* descriptorLayoutController_;
    //
    //ResourceKey sceneColorBuffer_;
    
    
    CustomTempBlurPassDesc desc;
    desc.root_ = this;
    desc.table_ = loader_->table_.get();
    desc.device_ = loader_->device_.get();
    desc.resourceProxy_ = resourceProxy_;
    desc.shaderModuleFactory_ = loader_->shaderModuleFactory_.get();
    desc.pipelineFactory_ = pipelineFactory_;
    desc.descriptorLayoutController_ = loader_->descriptorLayoutController_.get();
    desc.swapchain_ = loader_->swapchain_.get();
    desc.sceneColorBuffer_ = sceneColorBuffer;

    renderPassMap_[key] = std::make_unique<CustomTempBlurPass>(desc);
}

void Root::DefineRenderPass(PassKey const& key, RootGraphicsPassDesc const& desc)
{
    GraphicsPassDesc passDesc;
    passDesc.root_ = this;
    passDesc.table_ = loader_->table_.get();
    passDesc.device_ = loader_->device_.get();
    passDesc.proxy_ = resourceProxy_;
    passDesc.renderPassController_ = renderPassController_;
    passDesc.pipelineFactory_ = pipelineFactory_;
    passDesc.descriptorLayoutController_ = loader_->descriptorLayoutController_.get();
    passDesc.framedDescriptorsHub_ = framedDescriptorsHub_;
    passDesc.imagesProvider_ = imagesProvider_;
    passDesc.colorAttachmentCount_ = desc.colorAttachmentsCount_;
    for (auto i = 0u; i < passDesc.colorAttachmentCount_; ++i) {
        VKW::ImageView* attachmentView = FindGlobalImage(desc.colorAttachments_[i].resourceKey_, 0);
        VKW::ImageResource* attachmentResource = resourceProxy_->GetResource(attachmentView->resource_);

        if (i > 0) {
            assert(passDesc.width_ == attachmentResource->width_ && "All attachments in pass must have same dimentions!");
            assert(passDesc.height_ == attachmentResource->height_ && "All attachments in pass must have same dimentions!");
        }
        
        passDesc.width_ = attachmentResource->width_;
        passDesc.height_ = attachmentResource->height_;

        passDesc.colorAttachments_[i].handle_ = globalImages_[desc.colorAttachments_[i].resourceKey_];
        passDesc.colorAttachments_[i].usage_ = desc.colorAttachments_[i].usage_;
    }
    passDesc.depthStencilAttachment_ = desc.depthStencilAttachment_.size() > 0 ? &globalImages_[desc.depthStencilAttachment_] : nullptr;


    renderPassMap_[key] = std::make_unique<GraphicsPass>(passDesc);
}

BasePass& Root::FindPass(PassKey const& key)
{
    return *(renderPassMap_[key]);
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
    vkwDesc.renderPass_ = reinterpret_cast<GraphicsPass&>(*renderPassMap_[desc.renderPass_]).VKWRenderPass();
    vkwDesc.depthStencilInfo_ = desc.depthStencilInfo_;
    vkwDesc.dynamicStatesFlags_ = desc.dynamicStateFlags_;
    vkwDesc.blendingState_ = desc.blendingState_;
    
    VKW::PipelineHandle const vkwPipelineHandle = pipelineFactory_->CreateGraphicsPipeline(vkwDesc);
    VKW::Pipeline const* vkwPipeline = pipelineFactory_->GetPipeline(vkwPipelineHandle);
    VKW::PipelineLayoutHandle const vkwPipelineLayoutHandle = vkwPipeline->layoutHandle;


    pipeline.layoutHandle_ = vkwPipelineLayoutHandle;
    pipeline.pipelineHandle_ = vkwPipelineHandle;

    pipeline.properties_.pipelineDynamicStateFlags_ = desc.dynamicStateFlags_;
}

void Root::DefineComputePipeline(PipelineKey const& key, ComputePipelineDesc const& desc)
{
    Pipeline& pipeline = pipelineMap_[key];
    pipeline.staticLayoutMembersCount_ = desc.layoutDesc_->staticMembersCount_;
    pipeline.instancedLayoutMembersCount_ = 0;

    VKW::PipelineLayoutDesc vkwLayoutDesc;
    vkwLayoutDesc.membersCount_ = desc.layoutDesc_->staticMembersCount_;
    
    std::uint32_t const staticMembersCount = desc.layoutDesc_->staticMembersCount_;
    for (std::uint32_t i = 0; i < staticMembersCount; ++i) {
        pipeline.staticLayoutKeys_[i] = desc.layoutDesc_->staticMembers_[i];
        SetLayout const& setLayout = FindSetLayout(desc.layoutDesc_->staticMembers_[i]);
        vkwLayoutDesc.members_[i] = setLayout.vkwSetLayoutHandle_;
    }

    VKW::ComputePipelineDesc vkwDesc;
    vkwDesc.optimized_ = desc.optimized_;
    vkwDesc.layoutDesc_ = &vkwLayoutDesc;
    vkwDesc.shaderStage_ = desc.shaderStage_;

    VKW::PipelineHandle vkwPipelineHandle = pipelineFactory_->CreateComputePipeline(vkwDesc);
    VKW::Pipeline* vkwPipeline = pipelineFactory_->GetPipeline(vkwPipelineHandle);

    pipeline.layoutHandle_ = vkwPipeline->layoutHandle;
    pipeline.pipelineHandle_ = vkwPipelineHandle;
    pipeline.properties_.pipelineDynamicStateFlags_ = 0;
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

void Root::Decorate_VKWProxyDescriptorWriteDesc_UniformBuffer(VKW::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, UniformBufferHandle bufferHandle)
{
    UniformBuffer const& uniformBuffer = FindUniformBuffer(bufferHandle);

    std::uint32_t framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i) {
        VKW::ProxyDescriptorWriteDesc::Union::PureBufferDesc& pureBufferDesc = writeDesc.frames_[i].pureBufferDesc_;
        
        VKW::BufferView* view = resourceProxy_->GetBufferView(uniformBuffer.proxyBufferViewHandle_, i);

        pureBufferDesc.pureBufferViewHandle_ = resourceProxy_->GetBufferViewHandle(uniformBuffer.proxyBufferViewHandle_, i);
        pureBufferDesc.offset_ = 0;
        //pureBufferDesc.offset_ = static_cast<std::uint32_t>(view->offset_);
        pureBufferDesc.size_ = static_cast<std::uint32_t>(view->size_);
    }
}

void Root::Decorate_VKWProxyDescriptorWriteDesc_Texture(VKW::ProxyDescriptorWriteDesc& writeDesc, std::uint32_t id, VKW::ProxyImageHandle proxyImageHandle)
{
    std::uint32_t const framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i) {
        VKW::ProxyDescriptorWriteDesc::Union::ImageDesc& imageDesc = writeDesc.frames_[i].imageDesc_;

        VKW::ImageViewHandle imageViewHandle = resourceProxy_->GetImageViewHandle(proxyImageHandle, i);
        
        imageDesc.imageViewHandle_ = imageViewHandle;
        imageDesc.sampler_ = resourceProxy_->GetDefaultSampler();
        imageDesc.layout_ = VK_IMAGE_LAYOUT_GENERAL;
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
                Decorate_VKWProxyDescriptorWriteDesc_UniformBuffer(descriptorsWriteDescs[k], k, uniformBufferHandle);
                owner.slots_[i].descriptorSet_.setMembers_[k].data_.uniformBuffer_.uniformBufferHandle_ = uniformBufferHandle;
                break;

            case VKW::DESCRIPTOR_TYPE_TEXTURE:
                VKW::ProxyImageHandle imageProxyHandle = FindGlobalImage(setMemberData.texture2D_.imageKey_);
                Decorate_VKWProxyDescriptorWriteDesc_Texture(descriptorsWriteDescs[k], k, imageProxyHandle);
                owner.slots_[i].descriptorSet_.setMembers_[k].data_.texture2D_.textureKey_ = setMemberData.texture2D_.imageKey_;
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

        GraphicsPass& pass = reinterpret_cast<GraphicsPass&>(*renderPassMap_[materialTemplate.perPassData_[i].passKey_]);
        pass.RegisterMaterialData(key, i, pipelineKey);
    }
}

RenderWorkItemHandle Root::ConstructRenderWorkItem(Pipeline& pipeline, RenderWorkItemDesc const& desc)
{
    pipeline.renderItems_.emplace_back();
    RenderWorkItem& item = pipeline.renderItems_.back();

    item.vertexBufferKey_ = desc.vertexBufferKey_;
    item.indexBufferKey_ = desc.indexBufferKey_;
    item.vertexCount_ = desc.vertexCount_;
    item.vertexBindOffset_ = desc.vertexBindOffset_;
    item.indexCount_ = desc.indexCount_;
    item.indexBindOffset_ = desc.indexBindOffset_;
    
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

RenderWorkItemHandle Root::ConstructRenderWorkItem(PipelineKey const& pipelineKey, RenderWorkItemDesc const& desc)
{
    return ConstructRenderWorkItem(FindPipeline(pipelineKey), desc);
}

void Root::ReleaseRenderWorkItem(PipelineKey const& pipelineKey, RenderWorkItemHandle handle)
{
    return ReleaseRenderWorkItem(FindPipeline(pipelineKey), handle);
}

void Root::ImageLayoutTransition(std::uint32_t context, std::uint32_t imagesCount, VkImage* images, VkImageLayout* targetLayouts)
{
    VkImageMemoryBarrier imageBarriers[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];

    for (std::uint32_t i = 0; i < imagesCount; ++i)
    {
        VkImageMemoryBarrier& imageBarrier = imageBarriers[i];
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.pNext = nullptr;
        imageBarrier.srcAccessMask = VK_FLAGS_NONE;
        imageBarrier.dstAccessMask = VK_FLAGS_NONE;
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = targetLayouts[i];
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = images[i];
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = 1;
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = 1;
    }

    VKW::WorkerFrameCommandReciever commandReciever = mainWorkerTemp_->StartExecutionFrame(context);

    VulkanFuncTable()->vkCmdPipelineBarrier(
        commandReciever.commandBuffer_,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        imagesCount, imageBarriers);
    
    mainWorkerTemp_->EndExecutionFrame(context);

    mainWorkerTemp_->ExecuteFrame(context, VK_NULL_HANDLE, false);

    VK_ASSERT(VulkanFuncTable()->vkDeviceWaitIdle(loader_->device_->Handle()));
}

void Root::CopyStagingBufferToGPUBuffer(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context)
{
    VKW::BufferView* srcView = FindGlobalBuffer(src, context);
    VKW::BufferView* dstView = FindGlobalBuffer(dst, context);
    
    VKW::BufferResource* srcBuffer = resourceProxy_->GetResource(srcView->providedBuffer_->bufferResource_);
    VKW::BufferResource* dstBuffer = resourceProxy_->GetResource(dstView->providedBuffer_->bufferResource_);
    
    VKW::WorkerFrameCommandReciever commandReciever = mainWorkerTemp_->StartExecutionFrame(context);

    VkBufferCopy copyInfo;
    copyInfo.srcOffset = srcView->offset_;
    copyInfo.dstOffset = dstView->offset_;
    copyInfo.size = dstView->size_;

    VkBufferMemoryBarrier barriers[2];

    VkBufferMemoryBarrier& srcBarrier = barriers[0];
    srcBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    srcBarrier.pNext = nullptr;
    srcBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.buffer = srcBuffer->handle_;
    srcBarrier.offset = srcView->offset_;
    srcBarrier.size = srcView->size_;

    VkBufferMemoryBarrier& dstBarrier = barriers[1];
    dstBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    dstBarrier.pNext = nullptr;
    dstBarrier.srcAccessMask = VK_FLAGS_NONE;
    dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.buffer = srcBuffer->handle_;
    dstBarrier.offset = srcView->offset_;
    dstBarrier.size = srcView->size_;

    

    VulkanFuncTable()->vkCmdPipelineBarrier(
        commandReciever.commandBuffer_,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        2, barriers,
        0, nullptr);

    VulkanFuncTable()->vkCmdCopyBuffer(commandReciever.commandBuffer_, srcBuffer->handle_, dstBuffer->handle_, 1, &copyInfo);

    mainWorkerTemp_->EndExecutionFrame(context);

    mainWorkerTemp_->ExecuteFrame(context, VK_NULL_HANDLE, false);

    VK_ASSERT(VulkanFuncTable()->vkDeviceWaitIdle(loader_->device_->Handle()));
}

void Root::CopyStagingBufferToGPUTexture(ResourceKey const& src, ResourceKey const& dst, std::uint32_t context)
{
    VKW::BufferView* srcView = FindGlobalBuffer(src, context);
    VKW::ImageView* dstView = FindGlobalImage(dst, context);

    VKW::BufferResource* srcBuffer = resourceProxy_->GetResource(srcView->providedBuffer_->bufferResource_);
    VKW::ImageResource* dstImage = resourceProxy_->GetResource(dstView->resource_);

    VKW::WorkerFrameCommandReciever commandReciever = mainWorkerTemp_->StartExecutionFrame(context);
    
    VkBufferImageCopy copyInfo;
    copyInfo.bufferOffset = srcView->offset_;
    copyInfo.bufferRowLength = dstImage->width_;
    copyInfo.bufferImageHeight = dstImage->height_;
    copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyInfo.imageSubresource.mipLevel = 0;
    copyInfo.imageSubresource.baseArrayLayer = 0;
    copyInfo.imageSubresource.layerCount = 1;
    copyInfo.imageOffset.x = 0;
    copyInfo.imageOffset.y = 0;
    copyInfo.imageOffset.z = 0;
    copyInfo.imageExtent.width = dstImage->width_;
    copyInfo.imageExtent.height = dstImage->height_;
    copyInfo.imageExtent.depth = 1;

    VkBufferMemoryBarrier srcBufferBarrier;
    srcBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    srcBufferBarrier.pNext = nullptr;
    srcBufferBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    srcBufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBufferBarrier.buffer = srcBuffer->handle_;
    srcBufferBarrier.offset = srcView->offset_;
    srcBufferBarrier.size = srcView->size_;

    VkImageMemoryBarrier dstImageBarrier;
    dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    dstImageBarrier.pNext = nullptr;
    dstImageBarrier.srcAccessMask = VK_FLAGS_NONE;
    dstImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstImageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstImageBarrier.image = dstImage->handle_;
    dstImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstImageBarrier.subresourceRange.baseArrayLayer = 0;
    dstImageBarrier.subresourceRange.layerCount = 1;
    dstImageBarrier.subresourceRange.baseMipLevel = 0;
    dstImageBarrier.subresourceRange.levelCount = 1;

    VulkanFuncTable()->vkCmdPipelineBarrier(
        commandReciever.commandBuffer_,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        1, &srcBufferBarrier,
        1, &dstImageBarrier);
                                                                                                                     // WARNING, WE NEED TRANSITION TO THIS LAYOUT
    VulkanFuncTable()->vkCmdCopyBufferToImage(commandReciever.commandBuffer_, srcBuffer->handle_, dstImage->handle_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

    mainWorkerTemp_->EndExecutionFrame(context);

    mainWorkerTemp_->ExecuteFrame(context, VK_NULL_HANDLE, false);

    VK_ASSERT(VulkanFuncTable()->vkDeviceWaitIdle(loader_->device_->Handle()));
}

VKW::ResourceRendererProxy* Root::ResourceProxy() const
{
    return resourceProxy_;
}

void Root::PushPass(PassKey const& key)
{
    renderGraphRootTemp_.emplace_back(key);
}

VKW::PresentationContext Root::AcquireNextPresentationContext()
{
    return presentationController_->AcquireNewPresentationContext();
}

VKW::WorkerFrameCommandReciever Root::BeginRenderGraph(VKW::PresentationContext const& presentationContext)
{
    return mainWorkerTemp_->StartExecutionFrame(presentationContext.contextId_);
}

void Root::IterateRenderGraph(VKW::PresentationContext const& presentationContext, VKW::WorkerFrameCommandReciever& commandReciever)
{
    std::uint32_t contextId = presentationContext.contextId_;

    std::uint32_t const passCount = static_cast<std::uint32_t>(renderGraphRootTemp_.size());
    for (auto i = 0u; i < passCount; ++i) {
        auto& pass = *renderPassMap_[renderGraphRootTemp_[i]];
        pass.Begin(contextId, &commandReciever);
        pass.Apply(contextId, &commandReciever);
        pass.End(contextId, &commandReciever);
    }
}

void Root::EndRenderGraph(VKW::PresentationContext const& presentationContext, VKW::WorkerFrameCommandReciever& commandReciever)
{
    std::uint32_t const contextId = presentationContext.contextId_;

    // we can do transfer here
    
    VKW::ImageView* colorBufferView = FindGlobalImage(GetDefaultSceneColorOutput(), contextId);
    VKW::ImageResource* colorBufferResource = resourceProxy_->GetResource(colorBufferView->resource_);
    VkImage colorBufferHandle = colorBufferResource->handle_; // this to transfer src then back to color attachment (maybe without the last one)
    VkImage swapchainImageHandle = loader_->swapchain_->Image(contextId).image_;


    VkImageMemoryBarrier barriers[3];
    barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[0].pNext = nullptr;
    barriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barriers[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[0].image = colorBufferHandle;
    barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barriers[0].subresourceRange.baseArrayLayer = 0;
    barriers[0].subresourceRange.layerCount = 1;
    barriers[0].subresourceRange.baseMipLevel = 0;
    barriers[0].subresourceRange.levelCount = 1;

    barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[1].pNext = nullptr;
    barriers[1].srcAccessMask = VK_FLAGS_NONE;
    barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barriers[1].oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[1].image = swapchainImageHandle;
    barriers[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barriers[1].subresourceRange.baseArrayLayer = 0;
    barriers[1].subresourceRange.layerCount = 1;
    barriers[1].subresourceRange.baseMipLevel = 0;
    barriers[1].subresourceRange.levelCount = 1;

    // this barrier goes after copy
    barriers[2].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[2].pNext = nullptr;
    barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barriers[2].dstAccessMask = VK_FLAGS_NONE;
    barriers[2].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barriers[2].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barriers[2].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[2].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[2].image = swapchainImageHandle;
    barriers[2].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barriers[2].subresourceRange.baseArrayLayer = 0;
    barriers[2].subresourceRange.layerCount = 1;
    barriers[2].subresourceRange.baseMipLevel = 0;
    barriers[2].subresourceRange.levelCount = 1;

    VulkanFuncTable()->vkCmdPipelineBarrier(
        commandReciever.commandBuffer_,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        2, barriers
    );


    VkImageBlit toSwapchainBlitDesc;
    toSwapchainBlitDesc.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toSwapchainBlitDesc.srcSubresource.mipLevel = 0;
    toSwapchainBlitDesc.srcSubresource.baseArrayLayer = 0;
    toSwapchainBlitDesc.srcSubresource.layerCount = 1;
    toSwapchainBlitDesc.srcOffsets[0] = VkOffset3D{ (std::int32_t)COLOR_BUFFER_THREESHOLD, (std::int32_t)COLOR_BUFFER_THREESHOLD, 0 };
    toSwapchainBlitDesc.srcOffsets[1] = VkOffset3D{ (std::int32_t)(colorBufferResource->width_ - COLOR_BUFFER_THREESHOLD), (std::int32_t)(colorBufferResource->height_ - COLOR_BUFFER_THREESHOLD), 1 };
    
    toSwapchainBlitDesc.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toSwapchainBlitDesc.dstSubresource.mipLevel = 0;
    toSwapchainBlitDesc.dstSubresource.baseArrayLayer = 0;
    toSwapchainBlitDesc.dstSubresource.layerCount = 1;
    toSwapchainBlitDesc.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toSwapchainBlitDesc.dstOffsets[1] = VkOffset3D{ (std::int32_t)loader_->swapchain_->Width(), (std::int32_t)loader_->swapchain_->Height(), 1 };

    VulkanFuncTable()->vkCmdBlitImage(
        commandReciever.commandBuffer_,
        colorBufferHandle,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapchainImageHandle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &toSwapchainBlitDesc,
        VK_FILTER_LINEAR
    );

    VulkanFuncTable()->vkCmdPipelineBarrier(
        commandReciever.commandBuffer_,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, barriers + 2
    );
    
    mainWorkerTemp_->EndExecutionFrame(contextId);
    VKW::WorkerFrameCompleteSemaphore renderingCompleteSemaphore = mainWorkerTemp_->ExecuteFrame(
        contextId, 
        presentationContext.contextPresentationCompleteSemaphore_);

    presentationController_->PresentContextId(contextId, renderingCompleteSemaphore);
}

}