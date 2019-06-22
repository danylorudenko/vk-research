#include "CustomTempBlurPass.hpp"
#include "Root.hpp"
#include "..\vk_interface\pipeline\ShaderModuleFactory.hpp"
#include "..\vk_interface\worker\Worker.hpp"
#include "..\vk_interface\Swapchain.hpp"

#include <utility>
#include <cstdint>

namespace Render
{

CustomTempBlurPass::CustomTempBlurPass()
    : root_{ nullptr }
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
    , mixFactorUniformBuffer_{ "mxub" }
    , horizontalBlurPipeline_{ "hblp" }
    , verticalBlurPipeline_{ "vblp" }
    , mixPipeline_{ "mixp" }
    , universalSetLayout_{ "bllt" }
    , mixSetLayout_{ "mxlt"}
{
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
    mixSetLayoutDesc.membersCount_ = 4;
    mixSetLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[0].binding_ = 0;
    mixSetLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[1].binding_ = 1;
    mixSetLayoutDesc.membersDesc_[2].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    mixSetLayoutDesc.membersDesc_[2].binding_ = 2;
    mixSetLayoutDesc.membersDesc_[3].type_ = VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mixSetLayoutDesc.membersDesc_[3].binding_ = 3;

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
    horizontalPipelineDesc.optimized_ = false;
    horizontalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    horizontalPipelineDesc.shaderStage_.shaderModuleHandle_ = hModuleHandle;

    ComputePipelineDesc verticalPipelineDesc;
    verticalPipelineDesc.optimized_ = false;
    verticalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    verticalPipelineDesc.shaderStage_.shaderModuleHandle_ = vModuleHandle;

    ComputePipelineDesc mixPipelineDesc;
    mixPipelineDesc.optimized_ = false;
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

    VKW::BufferViewDesc mixFactorBufferDesc;
    mixFactorBufferDesc.format_ = VK_FORMAT_UNDEFINED;
    mixFactorBufferDesc.size_ = 64;
    mixFactorBufferDesc.usage_ = VKW::BufferUsage::UNIFORM;
    root_->DefineGlobalBuffer(mixFactorUniformBuffer_, mixFactorBufferDesc);


    // transitioning pass image to neede IMAGE_LAYOUTs
    VkImage transitionImages[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];
    VkImageLayout imageLayouts[VKW::CONSTANTS::MAX_FRAMES_BUFFERING];

    std::uint32_t const framesCount = resourceProxy_->FramesCount();
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        VKW::ImageView* vBufferImageView = root_->FindGlobalImage(verticalBlurBuffer_, i);
        VKW::ImageResource* vBufferImageResource = resourceProxy_->GetResource(vBufferImageView->resource_);
        transitionImages[i] = vBufferImageResource->handle_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    root_->ImageLayoutTransition(0, framesCount, transitionImages, imageLayouts);
    
    for (std::uint32_t i = 0; i < framesCount; ++i)
    {
        VKW::ImageView* hBufferImageView = root_->FindGlobalImage(horizontalBlurBuffer_, i);
        VKW::ImageResource* hBufferImageResource = resourceProxy_->GetResource(hBufferImageView->resource_);
        transitionImages[i] = hBufferImageResource->handle_;
        imageLayouts[i] = VK_IMAGE_LAYOUT_GENERAL;
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

    VKW::ProxyDescriptorWriteDesc mixSetDesc[4];
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

        VKW::ProxyBufferHandle mixFactorBufferProxyHandle = root_->FindGlobalBuffer(mixFactorUniformBuffer_);
        VKW::BufferViewHandle mixFactorBufferHandle = resourceProxy_->GetBufferViewHandle(mixFactorBufferProxyHandle, i);
        VKW::BufferView* mixFactorBufferView = resourceProxy_->GetBufferView(mixFactorBufferProxyHandle, i);
        mixSetDesc[3].frames_[i].pureBufferDesc_.pureBufferViewHandle_ = mixFactorBufferHandle;
        mixSetDesc[3].frames_[i].pureBufferDesc_.offset_ = 0;
        //mixSetDesc[3].frames_[i].pureBufferDesc_.offset_ = mixFactorBufferView->offset_;
        mixSetDesc[3].frames_[i].pureBufferDesc_.size_ = mixFactorBufferView->size_;
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

    std::swap(horizontalBlurPipeline_, rhs.horizontalBlurPipeline_);
    std::swap(verticalBlurPipeline_, rhs.verticalBlurPipeline_);

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

    constexpr std::uint32_t COMPUTE_LOCAL_GROUP_SIZE = 10;
    VkCommandBuffer cmdBuffer = commandReciever->commandBuffer_;

    VkPipeline hPipleineHandle = vkwHPipeline->vkPipeline_;
    VkPipeline vPipleineHandle = vkwVPipeline->vkPipeline_;

    VkImage colorBufferHandle = sceneColorBufferResource->handle_;
    VkImage horizontalBufferHandle = horizontalBufferResource->handle_;
    VkImage verticalBufferHandle = verticalBufferResource->handle_;
    VkImage swapchainBufferHandle = swapchain_->Image(contextId).image_;

    VkDescriptorSetLayout descriptorLayoutHandle = vkwDescriptorLayout->handle_;
    VkPipelineLayout hPipelineLayout = vkwHPipelineLayout->handle_;
    VkPipelineLayout vPipelineLayout = vkwVPipelineLayout->handle_;
    VkDescriptorSet hDescriptorSetHandle = vkwHDescriptorSet->handle_;
    VkDescriptorSet vDescriptorSetHandle = vkwVDescriptorSet->handle_;

    std::uint32_t const colorBufferWidth = sceneColorBufferResource->width_;
    std::uint32_t const colorBufferHeight = sceneColorBufferResource->height_;


    // gettin' colorbuffer to read
    VkImageMemoryBarrier colorBarrierIn;
    colorBarrierIn.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBarrierIn.pNext = nullptr;
    colorBarrierIn.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorBarrierIn.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    colorBarrierIn.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorBarrierIn.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBarrierIn.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrierIn.image = colorBufferHandle;
    colorBarrierIn.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBarrierIn.subresourceRange.baseArrayLayer = 0;
    colorBarrierIn.subresourceRange.layerCount = 1;
    colorBarrierIn.subresourceRange.baseMipLevel = 0;
    colorBarrierIn.subresourceRange.levelCount = 1;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, &colorBarrierIn
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

    VkImageMemoryBarrier vBufferToGeneral;
    vBufferToGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vBufferToGeneral.pNext = nullptr;
    vBufferToGeneral.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vBufferToGeneral.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vBufferToGeneral.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vBufferToGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vBufferToGeneral.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vBufferToGeneral.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vBufferToGeneral.image = verticalBufferHandle;
    vBufferToGeneral.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vBufferToGeneral.subresourceRange.baseArrayLayer = 0;
    vBufferToGeneral.subresourceRange.layerCount = 1;
    vBufferToGeneral.subresourceRange.baseMipLevel = 0;
    vBufferToGeneral.subresourceRange.levelCount = 1;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_FLAGS_NONE,
        1, &hBufferBarrier,
        0, nullptr,
        1, &vBufferToGeneral
    );

    // vertical blur dispatch    
    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vPipleineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vPipelineLayout, 0, 1, &vDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);




    // prepare to copy vertical blur results to color buffer
    VkImageMemoryBarrier colorBufferToTransferDst;
    colorBufferToTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBufferToTransferDst.pNext = nullptr;
    colorBufferToTransferDst.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    colorBufferToTransferDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    colorBufferToTransferDst.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorBufferToTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    colorBufferToTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBufferToTransferDst.image = colorBufferHandle;
    colorBufferToTransferDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBufferToTransferDst.subresourceRange.baseArrayLayer = 0;
    colorBufferToTransferDst.subresourceRange.layerCount = 1;
    colorBufferToTransferDst.subresourceRange.baseMipLevel = 0;
    colorBufferToTransferDst.subresourceRange.levelCount = 1;

    VkImageMemoryBarrier vBufferToTransferSrc;
    vBufferToTransferSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vBufferToTransferSrc.pNext = nullptr;
    vBufferToTransferSrc.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vBufferToTransferSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vBufferToTransferSrc.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    vBufferToTransferSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vBufferToTransferSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vBufferToTransferSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vBufferToTransferSrc.image = verticalBufferHandle;
    vBufferToTransferSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vBufferToTransferSrc.subresourceRange.baseArrayLayer = 0;
    vBufferToTransferSrc.subresourceRange.layerCount = 1;
    vBufferToTransferSrc.subresourceRange.baseMipLevel = 0;
    vBufferToTransferSrc.subresourceRange.levelCount = 1;

    VkImageMemoryBarrier beforeBlitBarriers[2];
    beforeBlitBarriers[0] = colorBufferToTransferDst;
    beforeBlitBarriers[1] = vBufferToTransferSrc;

    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        2, beforeBlitBarriers
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
        verticalBufferHandle,
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
