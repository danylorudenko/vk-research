#include "CustomTempBlurPass.hpp"
#include "Root.hpp"
#include "..\vk_interface\pipeline\ShaderModuleFactory.hpp"
#include "..\vk_interface\worker\Worker.hpp"
#include "..\vk_interface\Swapchain.hpp"
#include "..\renderer\UniformBufferWriterProxy.hpp"
#include "..\application\ImGuiUserData.hpp"
#include "..\io\IOManager.hpp"

#include <utility>
#include <cstdint>

namespace Render
{

CustomTempBlurPass::CustomTempBlurPass()
    : root_{ nullptr }
    , ioManager_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
    , swapchain_{ nullptr }
{

}

CustomTempBlurPass::CustomTempBlurPass(CustomTempBlurPassDesc const& desc)
    : root_{ desc.root_ }
    , ioManager_{ desc.ioManager_ }
    , table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourceProxy_{ desc.resourceProxy_ }
    , shaderModuleFactory_{ desc.shaderModuleFactory_ }
    , pipelineFactory_{ desc.pipelineFactory_ }
    , descriptorLayoutController_{ desc.descriptorLayoutController_ }
    , sceneColorBuffer_{ desc.sceneColorBuffer_ }
    , swapchain_{ desc.swapchain_ }
    , horizontalBlurBuffer_{ "hblb" }
    , verticalBlurBuffer_{ "vblb" }
    , blurMaskTexture_{ "blmk" }
    , horizontalBlurPipeline_{ "hblp" }
    , verticalBlurPipeline_{ "vblp" }
    , mixPipeline_{ "mixp" }
    , universalSetLayout_{ "bllt" }
    , mixSetLayout_{ "mxlt"}
{
    // allocating necessary api objects

    VKW::DescriptorSetLayoutDesc setLayoutDesc;
    setLayoutDesc.stage_ = VKW::DescriptorStage::COMPUTE;
    setLayoutDesc.membersCount_ = 2;
    setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    setLayoutDesc.membersDesc_[0].binding_ = 0;
    setLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    setLayoutDesc.membersDesc_[1].binding_ = 1;

    root_->DefineSetLayout(universalSetLayout_, setLayoutDesc);

    VKW::DescriptorSetLayoutDesc mixSetLayoutDesc;
    mixSetLayoutDesc.stage_ = VKW::DescriptorStage::COMPUTE;
    mixSetLayoutDesc.membersCount_ = 5;
    mixSetLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[0].binding_ = 0;
    mixSetLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[1].binding_ = 1;
    mixSetLayoutDesc.membersDesc_[2].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[2].binding_ = 2;
    mixSetLayoutDesc.membersDesc_[3].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[3].binding_ = 3;
    mixSetLayoutDesc.membersDesc_[4].type_ = VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mixSetLayoutDesc.membersDesc_[4].binding_ = 4;

    root_->DefineSetLayout(mixSetLayout_, mixSetLayoutDesc);



    PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.staticMembersCount_ = 1;
    pipelineLayoutDesc.staticMembers_[0] = universalSetLayout_;

    PipelineLayoutDesc mixPipelineLayoutDesc;
    mixPipelineLayoutDesc.staticMembersCount_ = 1;
    mixPipelineLayoutDesc.staticMembers_[0] = mixSetLayout_;


    VKW::ShaderModuleDesc hModuleDesc;
    hModuleDesc.type_ = VKW::SHADER_MODULE_TYPE_COMPUTE;
    hModuleDesc.shaderPath_ = "shader-src\\blur_horizontal.comp.spv";

    VKW::ShaderModuleDesc vModuleDesc;
    vModuleDesc.type_ = VKW::SHADER_MODULE_TYPE_COMPUTE;
    vModuleDesc.shaderPath_ = "shader-src\\blur_vertical.comp.spv";

    VKW::ShaderModuleDesc mixModuleDesc;
    mixModuleDesc.type_ = VKW::SHADER_MODULE_TYPE_COMPUTE;
    mixModuleDesc.shaderPath_ = "shader-src\\blur_mix.comp.spv";

    VKW::ShaderModuleHandle hModuleHandle = shaderModuleFactory_->LoadModule(hModuleDesc);
    VKW::ShaderModuleHandle vModuleHandle = shaderModuleFactory_->LoadModule(vModuleDesc);
    VKW::ShaderModuleHandle mixModuleHandle = shaderModuleFactory_->LoadModule(mixModuleDesc);

    ComputePipelineDesc horizontalPipelineDesc;
    horizontalPipelineDesc.optimized_ = true;
    horizontalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    horizontalPipelineDesc.shaderStage_.shaderModuleHandle_ = hModuleHandle;

    ComputePipelineDesc verticalPipelineDesc;
    verticalPipelineDesc.optimized_ = true;
    verticalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    verticalPipelineDesc.shaderStage_.shaderModuleHandle_ = vModuleHandle;

    ComputePipelineDesc mixPipelineDesc;
    mixPipelineDesc.optimized_ = true;
    mixPipelineDesc.layoutDesc_ = &mixPipelineLayoutDesc;
    mixPipelineDesc.shaderStage_.shaderModuleHandle_ = mixModuleHandle;

    root_->DefineComputePipeline(horizontalBlurPipeline_, horizontalPipelineDesc);
    root_->DefineComputePipeline(verticalBlurPipeline_, verticalPipelineDesc);
    root_->DefineComputePipeline(mixPipeline_, mixPipelineDesc);

    VKW::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, 0);
    VKW::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    std::uint32_t const width = sceneColorBufferResource->width_;
    std::uint32_t const height = sceneColorBufferResource->height_;
    VkFormat const format = sceneColorBufferResource->format_;

    VKW::ImageViewDesc computeBuffersDesc;
    computeBuffersDesc.format_ = format;
    computeBuffersDesc.width_ = width;
    computeBuffersDesc.height_ = height;
    computeBuffersDesc.usage_ = VKW::ImageUsage::STORAGE_IMAGE;
    root_->DefineGlobalImage(horizontalBlurBuffer_, computeBuffersDesc);
    root_->DefineGlobalImage(verticalBlurBuffer_, computeBuffersDesc);

    mixFactorUniformBuffer_ = root_->AcquireUniformBuffer(8);


    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask0.png", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask1.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask2.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask3.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask4.png", Data::TEXTURE_VARIATION_GRAY);
    Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask5.png", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask6.png", Data::TEXTURE_VARIATION_GRAY);

    char const* maskUploadImage = "msku";
    VKW::ImageViewDesc maskUploadImageDesc;
    maskUploadImageDesc.format_ = VK_FORMAT_R8_UNORM;
    maskUploadImageDesc.usage_ = VKW::ImageUsage::UPLOAD_IMAGE;
    maskUploadImageDesc.width_ = maskTexture2DData.width_;
    maskUploadImageDesc.height_ = maskTexture2DData.height_;
    root_->DefineGlobalImage(maskUploadImage, maskUploadImageDesc);

    VKW::ImageView* maskUploadImageView = root_->FindGlobalImage(maskUploadImage, 0);
    VKW::ImageResource* maskUploadImageResource = resourceProxy_->GetResource(maskUploadImageView->resource_);

    VkImageSubresource maskSubresource;
    maskSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    maskSubresource.arrayLayer = 0;
    maskSubresource.mipLevel = 0;

    VkSubresourceLayout maskSubresourceLayout;

    table_->vkGetImageSubresourceLayout(device_->Handle(), maskUploadImageResource->handle_, &maskSubresource, &maskSubresourceLayout);

    std::uint8_t* mappedUploadBufferMask = reinterpret_cast<std::uint8_t*>(root_->MapImage(maskUploadImage, 0));
    std::uint8_t* textureData = reinterpret_cast<std::uint8_t*>(maskTexture2DData.textureData_.data());
    for (std::uint32_t row = 0; row < maskTexture2DData.height_; ++row) {
        void* rowStart = mappedUploadBufferMask + (row * maskSubresourceLayout.rowPitch) + maskSubresourceLayout.offset;
        std::uint32_t rowSize = maskTexture2DData.width_;
        std::memcpy(rowStart, textureData + maskTexture2DData.width_ * row, rowSize);

    }
    root_->FlushImage(maskUploadImage, 0);


    VKW::ImageView* colorBufferImageView = root_->FindGlobalImage(sceneColorBuffer_, 0);
    VKW::ImageResource* colorBufferResource = resourceProxy_->GetResource(colorBufferImageView->resource_);
    VKW::ImageViewDesc maskImageDesc;
    maskImageDesc.format_ = VK_FORMAT_R8_UNORM;
    maskImageDesc.width_ = colorBufferResource->width_;
    maskImageDesc.height_ = colorBufferResource->height_;
    maskImageDesc.usage_ = VKW::ImageUsage::STORAGE_IMAGE_READONLY;
    root_->DefineGlobalImage(blurMaskTexture_, maskImageDesc);
    root_->BlitImages(maskUploadImage, blurMaskTexture_, 0, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

    


    // transitioning pass image to neede IMAGE_LAYOUTs
    VkImage transitionImages[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    VkImageLayout imageLayouts[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];

    std::uint32_t const framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        VKW::ImageView* vBufferImageView = root_->FindGlobalImage(verticalBlurBuffer_, i);
        VKW::ImageResource* vBufferImageResource = resourceProxy_->GetResource(vBufferImageView->resource_);
        transitionImages[i] = vBufferImageResource->handle_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_GENERAL;
    }
    root_->ImageLayoutTransition(0, framesCount, transitionImages, imageLayouts);
    
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        VKW::ImageView* hBufferImageView = root_->FindGlobalImage(horizontalBlurBuffer_, i);
        VKW::ImageResource* hBufferImageResource = resourceProxy_->GetResource(hBufferImageView->resource_);
        transitionImages[i] = hBufferImageResource->handle_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    root_->ImageLayoutTransition(0, framesCount, transitionImages, imageLayouts);



    horizontalDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_);
    verticalDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_);
    mixDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(mixSetLayout_).vkwSetLayoutHandle_);
    
    // set for horizontal "subpass"
    VKW::ProxyDescriptorWriteDesc horizontalSetDesc[2];
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        // input
        VKW::ProxyImageHandle colorBufferImageHandle = root_->FindGlobalImage(sceneColorBuffer_);
        horizontalSetDesc[0].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(colorBufferImageHandle, i);
        horizontalSetDesc[0].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        // output
        VKW::ProxyImageHandle horizontalProxyImageHandle = root_->FindGlobalImage(horizontalBlurBuffer_);
        horizontalSetDesc[1].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(horizontalProxyImageHandle, i);
        horizontalSetDesc[1].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;
    }

    resourceProxy_->WriteSet(horizontalDescriptorSet_, horizontalSetDesc);

    // set for vertical "subpass"
    VKW::ProxyDescriptorWriteDesc verticalSetDesc[2];
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        // input
        VKW::ProxyImageHandle horizontalProxyImageHandle = root_->FindGlobalImage(horizontalBlurBuffer_);
        verticalSetDesc[0].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(horizontalProxyImageHandle, i);
        verticalSetDesc[0].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        // output
        VKW::ProxyImageHandle verticalProxyImageHandle = root_->FindGlobalImage(verticalBlurBuffer_);
        verticalSetDesc[1].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(verticalProxyImageHandle, i);
        verticalSetDesc[1].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;
    }

    resourceProxy_->WriteSet(verticalDescriptorSet_, verticalSetDesc);

    VKW::ProxyDescriptorWriteDesc mixSetDesc[5];
    for (std::uint32_t i = 0; i < framesCount; ++i) 
    {
        // blurred input
        VKW::ProxyImageHandle verticalProxyImageHandle = root_->FindGlobalImage(verticalBlurBuffer_);
        mixSetDesc[0].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(verticalProxyImageHandle, i);
        mixSetDesc[0].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        // color input
        VKW::ProxyImageHandle colorBufferImageHandle = root_->FindGlobalImage(sceneColorBuffer_);
        mixSetDesc[1].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(colorBufferImageHandle, i);
        mixSetDesc[1].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        // output
        VKW::ProxyImageHandle horizontalProxyImageHandle = root_->FindGlobalImage(horizontalBlurBuffer_);
        mixSetDesc[2].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(horizontalProxyImageHandle, i);
        mixSetDesc[2].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        VKW::ProxyImageHandle blurMaskProxyImageHandle = root_->FindGlobalImage(blurMaskTexture_);
        mixSetDesc[3].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(blurMaskProxyImageHandle, i);
        mixSetDesc[3].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        UniformBuffer& mixFactorUniformBuffer = root_->FindUniformBuffer(mixFactorUniformBuffer_);
        VKW::BufferViewHandle mixFactorBufferHandle = resourceProxy_->GetBufferViewHandle(mixFactorUniformBuffer.proxyBufferViewHandle_, i);
        VKW::BufferView* mixFactorBufferView = resourceProxy_->GetBufferView(mixFactorUniformBuffer.proxyBufferViewHandle_, i);
        mixSetDesc[4].frames_[i].pureBufferDesc_.pureBufferViewHandle_ = mixFactorBufferHandle;
        mixSetDesc[4].frames_[i].pureBufferDesc_.offset_ = 0;
        mixSetDesc[4].frames_[i].pureBufferDesc_.size_ = (std::uint32_t)mixFactorBufferView->size_;
    }

    resourceProxy_->WriteSet(mixDescriptorSet_, mixSetDesc);
}

CustomTempBlurPass::CustomTempBlurPass(CustomTempBlurPass&& rhs)
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
    , swapchain_{ nullptr }
{
    operator=(std::move(rhs));
}

CustomTempBlurPass& CustomTempBlurPass::operator=(CustomTempBlurPass&& rhs)
{
    std::swap(root_, rhs.root_);
    std::swap(ioManager_, rhs.ioManager_);

    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(shaderModuleFactory_, rhs.shaderModuleFactory_);
    std::swap(pipelineFactory_, rhs.pipelineFactory_);
    std::swap(descriptorLayoutController_, rhs.descriptorLayoutController_);
    std::swap(swapchain_, rhs.swapchain_);

    std::swap(sceneColorBuffer_, rhs.sceneColorBuffer_);

    std::swap(horizontalBlurBuffer_, rhs.horizontalBlurBuffer_);
    std::swap(verticalBlurBuffer_, rhs.verticalBlurBuffer_);
    std::swap(blurMaskTexture_, rhs.blurMaskTexture_);

    std::swap(horizontalBlurPipeline_, rhs.horizontalBlurPipeline_);
    std::swap(verticalBlurPipeline_, rhs.verticalBlurPipeline_);
    std::swap(mixPipeline_, rhs.mixPipeline_);

    std::swap(universalSetLayout_, rhs.universalSetLayout_);
    std::swap(mixSetLayout_, rhs.mixSetLayout_);

    std::swap(horizontalDescriptorSet_, rhs.horizontalDescriptorSet_);
    std::swap(verticalDescriptorSet_, rhs.verticalDescriptorSet_);
    std::swap(mixDescriptorSet_, rhs.mixDescriptorSet_);

    std::swap(mixFactorUniformBuffer_, rhs.mixFactorUniformBuffer_);

    return *this;
}

CustomTempBlurPass::~CustomTempBlurPass()
{

}

void CustomTempBlurPass::Begin(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

void CustomTempBlurPass::Apply(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{
    Pipeline& hPipeline = root_->FindPipeline(horizontalBlurPipeline_);
    VKW::Pipeline* vkwHPipeline = pipelineFactory_->GetPipeline(hPipeline.pipelineHandle_);
    VKW::PipelineLayout* vkwHPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwHPipeline->layoutHandle);

    Pipeline& vPipeline = root_->FindPipeline(verticalBlurPipeline_);
    VKW::Pipeline* vkwVPipeline = pipelineFactory_->GetPipeline(vPipeline.pipelineHandle_);
    VKW::PipelineLayout* vkwVPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwVPipeline->layoutHandle);

    Pipeline& mixPipeline = root_->FindPipeline(mixPipeline_);
    VKW::Pipeline* vkwMixPipeline = pipelineFactory_->GetPipeline(mixPipeline.pipelineHandle_);
    VKW::PipelineLayout* vkwMixPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwMixPipeline->layoutHandle);


    VKW::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, contextId);
    VKW::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    VKW::ImageView* horizontalBufferView = root_->FindGlobalImage(horizontalBlurBuffer_, contextId);
    VKW::ImageResource* horizontalBufferResource = resourceProxy_->GetResource(horizontalBufferView->resource_);

    VKW::ImageView* verticalBufferView = root_->FindGlobalImage(verticalBlurBuffer_, contextId);
    VKW::ImageResource* verticalBufferResource = resourceProxy_->GetResource(verticalBufferView->resource_);

    VKW::DescriptorSetLayoutHandle vkwDescriptorLayoutHandle = root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_;
    VKW::DescriptorSetLayout* vkwDescriptorLayout = descriptorLayoutController_->GetDescriptorSetLayout(vkwDescriptorLayoutHandle);

    VKW::DescriptorSet* vkwHDescriptorSet = resourceProxy_->GetDescriptorSet(horizontalDescriptorSet_, contextId);
    VKW::DescriptorSet* vkwVDescriptorSet = resourceProxy_->GetDescriptorSet(verticalDescriptorSet_, contextId);
    VKW::DescriptorSet* vkwMixDescriptorSet = resourceProxy_->GetDescriptorSet(mixDescriptorSet_, contextId);

    constexpr std::uint32_t COMPUTE_LOCAL_GROUP_SIZE = 10;
    VkCommandBuffer cmdBuffer = commandReciever->commandBuffer_;

    VkPipeline hPipleineHandle = vkwHPipeline->vkPipeline_;
    VkPipeline vPipleineHandle = vkwVPipeline->vkPipeline_;
    VkPipeline mixPipelineHandle = vkwMixPipeline->vkPipeline_;

    VkImage colorBufferHandle = sceneColorBufferResource->handle_;
    VkImage horizontalBufferHandle = horizontalBufferResource->handle_;
    VkImage verticalBufferHandle = verticalBufferResource->handle_;
    VkImage swapchainBufferHandle = swapchain_->Image(contextId).image_;

    VkDescriptorSetLayout descriptorLayoutHandle = vkwDescriptorLayout->handle_;
    VkPipelineLayout hPipelineLayout = vkwHPipelineLayout->handle_;
    VkPipelineLayout vPipelineLayout = vkwVPipelineLayout->handle_;
    VkPipelineLayout mixPipelineLayout = vkwMixPipelineLayout->handle_;
    VkDescriptorSet hDescriptorSetHandle = vkwHDescriptorSet->handle_;
    VkDescriptorSet vDescriptorSetHandle = vkwVDescriptorSet->handle_;
    VkDescriptorSet mixDescriptorSetHandle = vkwMixDescriptorSet->handle_;

    std::uint32_t const colorBufferWidth = sceneColorBufferResource->width_;
    std::uint32_t const colorBufferHeight = sceneColorBufferResource->height_;

    // writing blur mix data
    void* mappedMixFactorUniformBuffer = root_->MapUniformBuffer(mixFactorUniformBuffer_, contextId);
    std::memcpy(mappedMixFactorUniformBuffer, &IMGUI_USER_BLUR_SCALE, sizeof(IMGUI_USER_BLUR_SCALE));
    root_->FlushUniformBuffer(mixFactorUniformBuffer_, contextId);
    



    // gettin' colorbuffer to read
    VkImageMemoryBarrier colorBarrierIn_hBufferToGeneral[2];
    colorBarrierIn_hBufferToGeneral[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBarrierIn_hBufferToGeneral[0].pNext = nullptr;
    colorBarrierIn_hBufferToGeneral[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorBarrierIn_hBufferToGeneral[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    colorBarrierIn_hBufferToGeneral[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorBarrierIn_hBufferToGeneral[0].newLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBarrierIn_hBufferToGeneral[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn_hBufferToGeneral[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn_hBufferToGeneral[0].image = colorBufferHandle;
    colorBarrierIn_hBufferToGeneral[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBarrierIn_hBufferToGeneral[0].subresourceRange.baseArrayLayer = 0;
    colorBarrierIn_hBufferToGeneral[0].subresourceRange.layerCount = 1;
    colorBarrierIn_hBufferToGeneral[0].subresourceRange.baseMipLevel = 0;
    colorBarrierIn_hBufferToGeneral[0].subresourceRange.levelCount = 1;

    colorBarrierIn_hBufferToGeneral[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBarrierIn_hBufferToGeneral[1].pNext = nullptr;
    colorBarrierIn_hBufferToGeneral[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    colorBarrierIn_hBufferToGeneral[1].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    colorBarrierIn_hBufferToGeneral[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    colorBarrierIn_hBufferToGeneral[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBarrierIn_hBufferToGeneral[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn_hBufferToGeneral[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn_hBufferToGeneral[1].image = horizontalBufferHandle;
    colorBarrierIn_hBufferToGeneral[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBarrierIn_hBufferToGeneral[1].subresourceRange.baseArrayLayer = 0;
    colorBarrierIn_hBufferToGeneral[1].subresourceRange.layerCount = 1;
    colorBarrierIn_hBufferToGeneral[1].subresourceRange.baseMipLevel = 0;
    colorBarrierIn_hBufferToGeneral[1].subresourceRange.levelCount = 1;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        2, colorBarrierIn_hBufferToGeneral
    );

    // horizontal blur
    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, hPipleineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, hPipelineLayout, 0, 1, &hDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);


    // getting horizontal blur results for vertical blur
    VkMemoryBarrier hBufferBarrier;
    hBufferBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    hBufferBarrier.pNext = nullptr;
    hBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    hBufferBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_FLAGS_NONE,
        1, &hBufferBarrier,
        0, nullptr,
        0, nullptr
    );

    // vertical blur dispatch    
    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vPipleineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vPipelineLayout, 0, 1, &vDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);

    
    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        0, nullptr
    );

    // mix
    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mixPipelineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mixPipelineLayout, 0, 1, &mixDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);



    // prepare to copy vertical blur results to color buffer
    VkImageMemoryBarrier colorBufferToTransferDst_hBufferToTransferSrc[2];
    colorBufferToTransferDst_hBufferToTransferSrc[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBufferToTransferDst_hBufferToTransferSrc[0].pNext = nullptr;
    colorBufferToTransferDst_hBufferToTransferSrc[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBufferToTransferDst_hBufferToTransferSrc[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    colorBufferToTransferDst_hBufferToTransferSrc[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst_hBufferToTransferSrc[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst_hBufferToTransferSrc[0].image = colorBufferHandle;
    colorBufferToTransferDst_hBufferToTransferSrc[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[0].subresourceRange.baseArrayLayer = 0;
    colorBufferToTransferDst_hBufferToTransferSrc[0].subresourceRange.layerCount = 1;
    colorBufferToTransferDst_hBufferToTransferSrc[0].subresourceRange.baseMipLevel = 0;
    colorBufferToTransferDst_hBufferToTransferSrc[0].subresourceRange.levelCount = 1;

    colorBufferToTransferDst_hBufferToTransferSrc[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBufferToTransferDst_hBufferToTransferSrc[1].pNext = nullptr;
    colorBufferToTransferDst_hBufferToTransferSrc[1].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[1].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBufferToTransferDst_hBufferToTransferSrc[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    colorBufferToTransferDst_hBufferToTransferSrc[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst_hBufferToTransferSrc[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst_hBufferToTransferSrc[1].image = horizontalBufferHandle;
    colorBufferToTransferDst_hBufferToTransferSrc[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBufferToTransferDst_hBufferToTransferSrc[1].subresourceRange.baseArrayLayer = 0;
    colorBufferToTransferDst_hBufferToTransferSrc[1].subresourceRange.layerCount = 1;
    colorBufferToTransferDst_hBufferToTransferSrc[1].subresourceRange.baseMipLevel = 0;
    colorBufferToTransferDst_hBufferToTransferSrc[1].subresourceRange.levelCount = 1;


    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        2, colorBufferToTransferDst_hBufferToTransferSrc
    );

    // putting results to color buffer
    VkImageBlit toColorBlitDesc;
    toColorBlitDesc.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColorBlitDesc.srcSubresource.mipLevel = 0;
    toColorBlitDesc.srcSubresource.baseArrayLayer = 0;
    toColorBlitDesc.srcSubresource.layerCount = 1;
    toColorBlitDesc.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toColorBlitDesc.srcOffsets[1] = VkOffset3D{ (std::int32_t)verticalBufferResource->width_, (std::int32_t)verticalBufferResource->height_, 1 };


    toColorBlitDesc.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColorBlitDesc.dstSubresource.mipLevel = 0;
    toColorBlitDesc.dstSubresource.baseArrayLayer = 0;
    toColorBlitDesc.dstSubresource.layerCount = 1;
    toColorBlitDesc.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toColorBlitDesc.dstOffsets[1] = VkOffset3D{ (std::int32_t)sceneColorBufferResource->width_, (std::int32_t)sceneColorBufferResource->height_, 1 };

    table_->vkCmdBlitImage(
        cmdBuffer,
        horizontalBufferHandle,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        colorBufferHandle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &toColorBlitDesc,
        VK_FILTER_NEAREST
    );

    // letting other passes use color buffer as a color attachment
    VkImageMemoryBarrier colorToAttachment;
    colorToAttachment.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorToAttachment.pNext = nullptr;
    colorToAttachment.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    colorToAttachment.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorToAttachment.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    colorToAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorToAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorToAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorToAttachment.image = colorBufferHandle;
    colorToAttachment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorToAttachment.subresourceRange.baseArrayLayer = 0;
    colorToAttachment.subresourceRange.layerCount = 1;
    colorToAttachment.subresourceRange.baseMipLevel = 0;
    colorToAttachment.subresourceRange.levelCount = 1;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, &colorToAttachment
    );
}

void CustomTempBlurPass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

}
