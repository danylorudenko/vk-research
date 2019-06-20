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
,     swapchain_{ nullptr }
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
    , horizontalBlurPipeline_{ "hblp" }
    , verticalBlurPipeline_{ "vblp" }
    , universalSetLayout_{ "bllt" }
{
    VKW::DescriptorSetLayoutDesc setLayoutDesc;
    setLayoutDesc.stage_ = VKW::DescriptorStage::COMPUTE;
    setLayoutDesc.membersCount_ = 2;
    setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    setLayoutDesc.membersDesc_[0].binding_ = 0;
    setLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_STORAGE_IMAGE;
    setLayoutDesc.membersDesc_[1].binding_ = 1;

    root_->DefineSetLayout(universalSetLayout_, setLayoutDesc);


    PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.staticMembersCount_ = 1;
    pipelineLayoutDesc.staticMembers_[0] = universalSetLayout_;


    VKW::ShaderModuleDesc hModuleDesc;
    hModuleDesc.type_ = VKW::SHADER_MODULE_TYPE_COMPUTE;
    hModuleDesc.shaderPath_ = "shader-src\\blur_horizontal.comp.spv";

    VKW::ShaderModuleDesc vModuleDesc;
    vModuleDesc.type_ = VKW::SHADER_MODULE_TYPE_COMPUTE;
    vModuleDesc.shaderPath_ = "shader-src\\blur_vertical.comp.spv";

    VKW::ShaderModuleHandle hModuleHandle = shaderModuleFactory_->LoadModule(hModuleDesc);
    VKW::ShaderModuleHandle vModuleHandle = shaderModuleFactory_->LoadModule(vModuleDesc);

    ComputePipelineDesc horizontalPipelineDesc;
    horizontalPipelineDesc.optimized_ = false;
    horizontalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    horizontalPipelineDesc.shaderStage_.shaderModuleHandle_ = hModuleHandle;

    ComputePipelineDesc verticalPipelineDesc;
    verticalPipelineDesc.optimized_ = false;
    verticalPipelineDesc.layoutDesc_ = &pipelineLayoutDesc;
    verticalPipelineDesc.shaderStage_.shaderModuleHandle_ = vModuleHandle;

    root_->DefineComputePipeline(horizontalBlurPipeline_, horizontalPipelineDesc);
    root_->DefineComputePipeline(verticalBlurPipeline_, verticalPipelineDesc);

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
    
    horizontalDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_);
    verticalDescriptorSet_ = resourceProxy_->CreateSet(root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_);
    
    // set for horizontal "subpass"
    std::uint32_t const framesCount = resourceProxy_->FramesCount();
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

    Pipeline& vPipeline = root_->FindPipeline(verticalBlurPipeline_);
    VKW::Pipeline* vkwVPipeline = pipelineFactory_->GetPipeline(vPipeline.pipelineHandle_);


    VKW::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, contextId);
    VKW::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    VKW::ImageView* horizontalBufferView = root_->FindGlobalImage(horizontalBlurBuffer_, contextId);
    VKW::ImageResource* horizontalBufferResource = resourceProxy_->GetResource(horizontalBufferView);

    VKW::ImageView* verticalBufferView = root_->FindGlobalImage(verticalBlurBuffer_, contextId);
    VKW::ImageResource* verticalBufferResource = resourceProxy_->GetResource(verticalBufferView);

    VKW::DescriptorSetLayoutHandle vkwDescriptorLayoutHandle = root_->FindSetLayout(universalSetLayout_).vkwSetLayoutHandle_;
    VKW::DescriptorSetLayout* vkwDescriptorLayout = descriptorLayoutController_->GetDescriptorSetLayout();

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
    VkDescriptorSet hDescriptorSetHandle = vkwHDescriptorSet->handle_;
    VkDescriptorSet vDescriptorSetHandle = vkwVDescriptorSet->handle_;

    std::uint32_t const colorBufferWidth = sceneColorBufferResource->width_;
    std::uint32_t const colorBufferHeight = sceneColorBufferResource->height_;

#pragma region barriersin
    
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
        1, colorBarrierIn
    );

    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, hPipleineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, descriptorLayoutHandle, 0, 1, &hDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);



    VkMemoryBarrier hBufferBarrier;
    hBufferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
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


    table_->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vPipleineHandle);
    table_->vkCmdBindDescriptorSets(cmdBuffer, descriptorLayoutHandle, 0, 1, &vDescriptorSetHandle, 0, nullptr);
    table_->vkCmdDispatch(cmdBuffer, colorBufferWidth / COMPUTE_LOCAL_GROUP_SIZE, colorBufferHeight / COMPUTE_LOCAL_GROUP_SIZE, 1);



    VkMemoryBarrier vBufferBarrier;
    vBufferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vBufferBarrier.pNext = nullptr;
    vBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vBufferBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;


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
    
    table_->vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_FLAGS_NONE,
        1, &vBufferBarrier,
        0, nullptr,
        1, &colorBufferToTransferDst
    );


    VkImageBlit toColorBlitDesc;
    toColorBlitDesc.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColorBlitDesc.srcSubresource.mipLevel = 0;
    toColorBlitDesc.srcSubresource.baseArrayLayer = 0;
    toColorBlitDesc.srcSubresource.layerCount = 1;
    toColorBlitDesc.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toColorBlitDesc.srcOffsets[1] = VkOffset3D{ (std::int32_t)sceneColorBufferResource->width_, (std::int32_t)sceneColorBufferResource->height_, 1 };


    toColorBlitDesc.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColorBlitDesc.dstSubresource.mipLevel = 0;
    toColorBlitDesc.dstSubresource.baseArrayLayer = 0;
    toColorBlitDesc.dstSubresource.layerCount = 1;
    toColorBlitDesc.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
    toColorBlitDesc.dstOffsets[1] = VkOffset3D{ (std::int32_t)swapchain_->Width(), (std::int32_t)swapchain_->Height(), 1 };

    table_->vkCmdBlitImage(
        cmdBuffer,
        verticalBufferHandle,
        VK_IMAGE_LAYOUT_GENERAL,
        colorBufferHandle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &toColorBlitDesc,
        VK_FILTER_NEAREST
    );

#pragma endregion barriersin

#pragma region blittoswapchain

    //VkImageBlit blitDesc;
    //blitDesc.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //blitDesc.srcSubresource.mipLevel = 0;
    //blitDesc.srcSubresource.baseArrayLayer = 0;
    //blitDesc.srcSubresource.layerCount = 1;
    //blitDesc.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
    //blitDesc.srcOffsets[1] = VkOffset3D{ (std::int32_t)sceneColorBufferResource->width_, (std::int32_t)sceneColorBufferResource->height_, 1 };
    //
    //
    //blitDesc.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //blitDesc.dstSubresource.mipLevel = 0;
    //blitDesc.dstSubresource.baseArrayLayer = 0;
    //blitDesc.dstSubresource.layerCount = 1;
    //blitDesc.dstOffsets[0] = VkOffset3D{ 0, 0, 0 };
    //blitDesc.dstOffsets[1] = VkOffset3D{ (std::int32_t)swapchain_->Width(), (std::int32_t)swapchain_->Height(), 1 };
    //
    //
    //table_->vkCmdBlitImage(
    //    cmdBuffer,
    //    source,
    //    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //    dest,
    //    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //    1, &blitDesc,
    //    VK_FILTER_NEAREST
    //);
#pragma endregion blittoswapchain


#pragma region barriersout

    //VkImageMemoryBarrier colorBarrierOut;
    //colorBarrierOut.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //colorBarrierOut.pNext = nullptr;
    //colorBarrierOut.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    //colorBarrierOut.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //colorBarrierOut.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    //colorBarrierOut.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //colorBarrierOut.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //colorBarrierOut.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //colorBarrierOut.image = source;
    //colorBarrierOut.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //colorBarrierOut.subresourceRange.baseArrayLayer = 0;
    //colorBarrierOut.subresourceRange.layerCount = 1;
    //colorBarrierOut.subresourceRange.baseMipLevel = 0;
    //colorBarrierOut.subresourceRange.levelCount = 1;
    //
    //VkImageMemoryBarrier swapchainOut;
    //swapchainOut.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //swapchainOut.pNext = nullptr;
    //swapchainOut.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //swapchainOut.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //swapchainOut.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    //swapchainOut.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //swapchainOut.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //swapchainOut.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //swapchainOut.image = dest;
    //swapchainOut.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //swapchainOut.subresourceRange.baseArrayLayer = 0;
    //swapchainOut.subresourceRange.layerCount = 1;
    //swapchainOut.subresourceRange.baseMipLevel = 0;
    //swapchainOut.subresourceRange.levelCount = 1;
    //
    //VkImageMemoryBarrier outBarriers[2] = { colorBarrierOut, swapchainOut };
    //
    //table_->vkCmdPipelineBarrier(
    //    cmdBuffer,
    //    VK_PIPELINE_STAGE_TRANSFER_BIT,
    //    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //    VK_FLAGS_NONE,
    //    0, nullptr,
    //    0, nullptr,
    //    2, outBarriers
    //);
#pragma endregion barriersout

    // 0 - we also need to fill descriptor sets
    
    // 1 - blur horzontally
    // 2 - blur vertically
    // 3 - blit to swapchain
}

void CustomTempBlurPass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

}
