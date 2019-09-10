#include "CustomTempBlurPass.hpp"
#include "Root.hpp"
#include "..\VAL\pipeline\ShaderModuleFactory.hpp"
#include "..\VAL\worker\Worker.hpp"
#include "..\VAL\Swapchain.hpp"
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
    , blurBuffer_{ "hblb" }
    , blurMaskTexture_{ "blmk" }
    , blurFastPipeline_{ "mixp" }
    , blurFullPipeline_{ "blfl"}
    , blurSetLayout_{ "mxlt"}
{
    // allocating necessary api objects
    VAL::DescriptorSetLayoutDesc blurSetLayoutDesc;
    blurSetLayoutDesc.stage_ = VAL::DescriptorStage::COMPUTE;
    blurSetLayoutDesc.membersCount_ = 4;
    blurSetLayoutDesc.membersDesc_[0].type_ = VAL::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    blurSetLayoutDesc.membersDesc_[0].binding_ = 0;
    blurSetLayoutDesc.membersDesc_[1].type_ = VAL::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    blurSetLayoutDesc.membersDesc_[1].binding_ = 1;
    blurSetLayoutDesc.membersDesc_[2].type_ = VAL::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    blurSetLayoutDesc.membersDesc_[2].binding_ = 2;
    blurSetLayoutDesc.membersDesc_[3].type_ = VAL::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    blurSetLayoutDesc.membersDesc_[3].binding_ = 3;

    root_->DefineSetLayout(blurSetLayout_, blurSetLayoutDesc);


    PipelineLayoutDesc mixPipelineLayoutDesc;
    mixPipelineLayoutDesc.staticMembersCount_ = 1;
    mixPipelineLayoutDesc.staticMembers_[0] = blurSetLayout_;


    VAL::ShaderModuleDesc blurFastModuleDesc;
    blurFastModuleDesc.type_ = VAL::SHADER_MODULE_TYPE_COMPUTE;
    blurFastModuleDesc.shaderPath_ = "shader-src\\blur_single_pass.comp.spv";

    VAL::ShaderModuleDesc blurFullModuleDesc;
    blurFullModuleDesc.type_ = VAL::SHADER_MODULE_TYPE_COMPUTE;
    blurFullModuleDesc.shaderPath_ = "shader-src\\blur_single_pass_heavy.comp.spv";


    VAL::ShaderModuleHandle blurFastModuleHandle = shaderModuleFactory_->LoadModule(blurFastModuleDesc);
    VAL::ShaderModuleHandle blurFullModuleHandle = shaderModuleFactory_->LoadModule(blurFullModuleDesc);

    ComputePipelineDesc blurFastPipelineDesc;
    blurFastPipelineDesc.optimized_ = true;
    blurFastPipelineDesc.layoutDesc_ = &mixPipelineLayoutDesc;
    blurFastPipelineDesc.shaderStage_.shaderModuleHandle_ = blurFastModuleHandle;

    ComputePipelineDesc blurFullPipelineDesc;
    blurFullPipelineDesc.optimized_ = true;
    blurFullPipelineDesc.layoutDesc_ = &mixPipelineLayoutDesc;
    blurFullPipelineDesc.shaderStage_.shaderModuleHandle_ = blurFullModuleHandle;

    root_->DefineComputePipeline(blurFastPipeline_, blurFastPipelineDesc);
    root_->DefineComputePipeline(blurFullPipeline_, blurFullPipelineDesc);

    VAL::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, 0);
    VAL::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    std::uint32_t const width = sceneColorBufferResource->width_;
    std::uint32_t const height = sceneColorBufferResource->height_;
    VkFormat const format = sceneColorBufferResource->format_;

    VAL::ImageViewDesc computeBuffersDesc;
    computeBuffersDesc.format_ = format;
    computeBuffersDesc.width_ = width;
    computeBuffersDesc.height_ = height;
    computeBuffersDesc.usage_ = VAL::ImageUsage::STORAGE_IMAGE;
    root_->DefineGlobalImage(blurBuffer_, computeBuffersDesc);

    mixFactorUniformBuffer_ = root_->AcquireUniformBuffer(8);


    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask0.png", Data::TEXTURE_VARIATION_GRAY);
    Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask1.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask2.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask3.jpg", Data::TEXTURE_VARIATION_GRAY);
    //Data::Texture2D maskTexture2DData = ioManager_->ReadTexture2D("textures\\mask4.png", Data::TEXTURE_VARIATION_GRAY);

    char const* maskUploadImage = "msku";
    VAL::ImageViewDesc maskUploadImageDesc;
    maskUploadImageDesc.format_ = VK_FORMAT_R8_UNORM;
    maskUploadImageDesc.usage_ = VAL::ImageUsage::UPLOAD_IMAGE;
    maskUploadImageDesc.width_ = maskTexture2DData.width_;
    maskUploadImageDesc.height_ = maskTexture2DData.height_;
    root_->DefineGlobalImage(maskUploadImage, maskUploadImageDesc);

    VAL::ImageView* maskUploadImageView = root_->FindGlobalImage(maskUploadImage, 0);
    VAL::ImageResource* maskUploadImageResource = resourceProxy_->GetResource(maskUploadImageView->resource_);

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


    VAL::ImageView* colorBufferImageView = root_->FindGlobalImage(sceneColorBuffer_, 0);
    VAL::ImageResource* colorBufferResource = resourceProxy_->GetResource(colorBufferImageView->resource_);
    VAL::ImageViewDesc maskImageDesc;
    maskImageDesc.format_ = VK_FORMAT_R8_UNORM;
    maskImageDesc.width_ = colorBufferResource->width_;
    maskImageDesc.height_ = colorBufferResource->height_;
    maskImageDesc.usage_ = VAL::ImageUsage::STORAGE_IMAGE_READONLY;
    root_->DefineGlobalImage(blurMaskTexture_, maskImageDesc);
    root_->BlitImages(maskUploadImage, blurMaskTexture_, 0, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

    


    // transitioning pass image to neede IMAGE_LAYOUTs
    VkImage transitionImages[VAL::CONSTANTS::MAX_FRAMES_BUFFERING];
    VkImageLayout imageLayouts[VAL::CONSTANTS::MAX_FRAMES_BUFFERING];

    std::uint32_t const framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        VAL::ImageView* blurBufferImageView = root_->FindGlobalImage(blurBuffer_, i);
        VAL::ImageResource* blurBufferImageResource = resourceProxy_->GetResource(blurBufferImageView->resource_);
        transitionImages[i] = blurBufferImageResource->handle_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    root_->ImageLayoutTransition(0, framesCount, transitionImages, imageLayouts);



    
    blurDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(blurSetLayout_).vkwSetLayoutHandle_);
    
    VAL::ProxyDescriptorWriteDesc blurSetDesc[5];
    for (std::uint32_t i = 0; i < framesCount; ++i) 
    {
        // color input
        VAL::ProxyImageHandle colorBufferImageHandle = root_->FindGlobalImage(sceneColorBuffer_);
        blurSetDesc[0].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(colorBufferImageHandle, i);
        blurSetDesc[0].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        // output
        VAL::ProxyImageHandle horizontalProxyImageHandle = root_->FindGlobalImage(blurBuffer_);
        blurSetDesc[1].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(horizontalProxyImageHandle, i);
        blurSetDesc[1].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        VAL::ProxyImageHandle blurMaskProxyImageHandle = root_->FindGlobalImage(blurMaskTexture_);
        blurSetDesc[2].frames_[i].imageDesc_.imageViewHandle_ = resourceProxy_->GetImageViewHandle(blurMaskProxyImageHandle, i);
        blurSetDesc[2].frames_[i].imageDesc_.layout_ = VK_IMAGE_LAYOUT_GENERAL;

        UniformBuffer& mixFactorUniformBuffer = root_->FindUniformBuffer(mixFactorUniformBuffer_);
        VAL::BufferViewHandle mixFactorBufferHandle = resourceProxy_->GetBufferViewHandle(mixFactorUniformBuffer.proxyBufferViewHandle_, i);
        VAL::BufferView* mixFactorBufferView = resourceProxy_->GetBufferView(mixFactorUniformBuffer.proxyBufferViewHandle_, i);
        blurSetDesc[3].frames_[i].pureBufferDesc_.pureBufferViewHandle_ = mixFactorBufferHandle;
        blurSetDesc[3].frames_[i].pureBufferDesc_.offset_ = 0;
        blurSetDesc[3].frames_[i].pureBufferDesc_.size_ = (std::uint32_t)mixFactorBufferView->size_;
    }

    resourceProxy_->WriteSet(blurDescriptorSet_, blurSetDesc);
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

    std::swap(blurBuffer_, rhs.blurBuffer_);
    std::swap(blurMaskTexture_, rhs.blurMaskTexture_);

    std::swap(blurFastPipeline_, rhs.blurFastPipeline_);
    std::swap(blurFullPipeline_, rhs.blurFullPipeline_);

    std::swap(blurSetLayout_, rhs.blurSetLayout_);

    std::swap(blurDescriptorSet_, rhs.blurDescriptorSet_);

    std::swap(mixFactorUniformBuffer_, rhs.mixFactorUniformBuffer_);

    return *this;
}

CustomTempBlurPass::~CustomTempBlurPass()
{

}

void CustomTempBlurPass::Begin(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever)
{

}

void CustomTempBlurPass::Apply(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever)
{
    Pipeline& blurFastPipeline = root_->FindPipeline(blurFastPipeline_);
    Pipeline& blurFullPipeline = root_->FindPipeline(blurFullPipeline_);
    VAL::Pipeline* vkwBlurFastPipeline = pipelineFactory_->GetPipeline(blurFastPipeline.pipelineHandle_);
    VAL::Pipeline* vkwBlurFullPipeline = pipelineFactory_->GetPipeline(blurFullPipeline.pipelineHandle_);
    VAL::PipelineLayout* vkwBlurFastPipelineLayout = descriptorLayoutController_->GetPipelineLayout(vkwBlurFastPipeline->layoutHandle);


    VAL::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, contextId);
    VAL::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    VAL::ImageView* horizontalBufferView = root_->FindGlobalImage(blurBuffer_, contextId);
    VAL::ImageResource* horizontalBufferResource = resourceProxy_->GetResource(horizontalBufferView->resource_);

    VAL::DescriptorSet* vkwBlurDescriptorSet = resourceProxy_->GetDescriptorSet(blurDescriptorSet_, contextId);

    constexpr std::uint32_t COMPUTE_LOCAL_GROUP_SIZE_X = 5;
    constexpr std::uint32_t COMPUTE_LOCAL_GROUP_SIZE_Y = 5;
    VkCommandBuffer cmdBuffer = commandReciever->commandBuffer_;

    VkPipeline blurFastPipelineHandle = vkwBlurFastPipeline->vkPipeline_;
    VkPipeline blurFullPipelineHandle = vkwBlurFullPipeline->vkPipeline_;

    VkImage colorBufferHandle = sceneColorBufferResource->handle_;
    VkImage horizontalBufferHandle = horizontalBufferResource->handle_;

    VkPipelineLayout mixPipelineLayout = vkwBlurFastPipelineLayout->handle_;
    VkDescriptorSet mixDescriptorSetHandle = vkwBlurDescriptorSet->handle_;

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
    colorBarrierIn_hBufferToGeneral[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
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

    // blur
    if(IMGUI_USER_BLUR == IMGUI_USER_BLUR_TYPE_FAST)
        table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, blurFastPipelineHandle);
    else
        table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, blurFullPipelineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mixPipelineLayout, 0, 1, &mixDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE_X, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE_Y, 1);



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
    colorBufferToTransferDst_hBufferToTransferSrc[1].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
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
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        2, colorBufferToTransferDst_hBufferToTransferSrc
    );
    
    //// putting results to color buffer
    VkImageBlit toColorBlitDesc;
    toColorBlitDesc.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColorBlitDesc.srcSubresource.mipLevel = 0;
    toColorBlitDesc.srcSubresource.baseArrayLayer = 0;
    toColorBlitDesc.srcSubresource.layerCount = 1;
    toColorBlitDesc.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toColorBlitDesc.srcOffsets[1] = VkOffset3D{ (std::int32_t)horizontalBufferResource->width_, (std::int32_t)horizontalBufferResource->height_, 1 };
    
    
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

void CustomTempBlurPass::End(std::uint32_t contextId, VAL::WorkerFrameCommandReciever* commandReciever)
{

}

}
