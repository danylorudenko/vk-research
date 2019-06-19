#include "CustomTempBlurPass.hpp"
#include "Root.hpp"
#include "..\vk_interface\pipeline\ShaderModuleFactory.hpp"

#include <utility>

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
    , horizontalBlurBuffer_{ "hblb" }
    , verticalBlurBuffer_{ "vblb" }
    , horizontalBlurPipeline_{ "hblp" }
    , verticalBlurPipeline_{ "vblp" }
    , universalSetLayout_{ "bllt" }
{
    VKW::DescriptorSetLayoutDesc setLayoutDesc;
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


    
    /////////////
    VKW::ImageView* sceneColorBufferView = root_->FindGlobalImage(sceneColorBuffer_, 0);
    VKW::ImageResource* sceneColorBufferResource = resourceProxy_->GetResource(sceneColorBufferView->resource_);

    std::uint32_t const width = sceneColorBufferResource->width_;
    std::uint32_t const height = sceneColorBufferResource->height_;

    VKW::ImageViewDesc imageViewDesc;
    //imageViewDesc.
}

CustomTempBlurPass::CustomTempBlurPass(CustomTempBlurPass&& rhs)
    : root_{ nullptr }
    , table_{ nullptr }
    , device_{ nullptr }
    , resourceProxy_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , pipelineFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
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
    // 1 - blur horzontally
    // 2 - blur vertically
    // 3 - blit to swapchain
}

void CustomTempBlurPass::End(std::uint32_t contextId, VKW::WorkerFrameCommandReciever* commandReciever)
{

}

}
