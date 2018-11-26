#include "PipelineFactory.hpp"

#include <utility>

#include "..\Device.hpp"
#include "..\ImportTable.hpp"
#include "RenderPassController.hpp"
#include "ShaderModuleFactory.hpp"

namespace VKW
{

PipelineFactory::PipelineFactory()
    : table_{ nullptr }
    , device_{ nullptr }
    , renderPassController_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
{

}

PipelineFactory::PipelineFactory(PipelineFactoryDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , renderPassController_{ desc.renderPassController_ }
    , shaderModuleFactory_{ desc.shaderModuleFactory_ }
    , descriptorLayoutController_{ desc.descriptorLayoutController_ }
{

}

PipelineFactory::PipelineFactory(PipelineFactory&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , renderPassController_{ nullptr }
    , shaderModuleFactory_{ nullptr }
    , descriptorLayoutController_{ nullptr }
{
    operator=(std::move(rhs));
}

PipelineFactory& PipelineFactory::operator=(PipelineFactory&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(renderPassController_, rhs.renderPassController_);
    std::swap(shaderModuleFactory_, rhs.shaderModuleFactory_);
    std::swap(descriptorLayoutController_, rhs.descriptorLayoutController_);

    std::swap(pipelines_, rhs.pipelines_);

    return *this;
}

PipelineFactory::~PipelineFactory()
{

}

VkShaderStageFlagBits VKWShaderTypeToVkFlags(ShaderModuleType type)
{
    switch (type) {

    case SHADER_MODULE_TYPE_VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case SHADER_MODULE_TYPE_FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case SHADER_MODULE_TYPE_COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;

    default:
        return static_cast<VkShaderStageFlagBits>(VK_FLAGS_NONE);
    }
}

PipelineHandle PipelineFactory::CreateGraphicsPipeline(GraphicsPipelineDesc const& desc)
{
    static VkPipelineShaderStageCreateInfo shaderStagesInfo[Pipeline::MAX_SHADER_STAGES];
    static VkVertexInputBindingDescription inputBindingsInfo[Pipeline::MAX_VERTEX_ATTRIBUTES];
    static VkVertexInputAttributeDescription inputAttributesInfo[Pipeline::MAX_VERTEX_ATTRIBUTES];
    static VkViewport viewports[Pipeline::MAX_VIEWPORTS];
    static VkRect2D scissorRects[Pipeline::MAX_VIEWPORTS];
    
    PipelineLayoutHandle layoutHandle = descriptorLayoutController_->CreatePipelineLayout(desc.layoutDesc_);
    PipelineLayout* layout = descriptorLayoutController_->GetPipelineLayout(layoutHandle);
    RenderPass* renderPass = renderPassController_->GetRenderPass(desc.renderPass_);

    static VkGraphicsPipelineCreateInfo graphicsPipelineInfo;
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.pNext = nullptr;
    graphicsPipelineInfo.flags = desc.optimized_ ? VK_FLAGS_NONE : VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineInfo.basePipelineIndex = 0;
    graphicsPipelineInfo.layout = layout->handle_;
    graphicsPipelineInfo.renderPass = renderPass->handle_;
    graphicsPipelineInfo.subpass = 0;



    graphicsPipelineInfo.stageCount = desc.shaderStagesCount_;
    for (auto i = 0u; i < desc.shaderStagesCount_; ++i) {
        ShaderModule* shModule = shaderModuleFactory_->AccessModule(desc.shaderStages_[i].shaderModuleHandle_);
        
        auto& vkShStage = shaderStagesInfo[i];
        vkShStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vkShStage.pNext = nullptr;
        vkShStage.module = shModule->handle_;
        vkShStage.pName = shModule->entryPoint_.c_str();
        vkShStage.stage = VKWShaderTypeToVkFlags(shModule->type_);
        vkShStage.flags = VK_FLAGS_NONE;
        vkShStage.pSpecializationInfo = nullptr;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState;
    {
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.pNext = nullptr;
        vertexInputState.flags = VK_FLAGS_NONE;

        auto const attrCount = desc.vertexInputInfo_->vertexAttributesCount_;
        vertexInputState.vertexBindingDescriptionCount = 1;
        vertexInputState.vertexAttributeDescriptionCount = attrCount;

        inputBindingsInfo[0].binding = desc.vertexInputInfo_->binding_;
        inputBindingsInfo[0].stride = desc.vertexInputInfo_->stride_;
        inputBindingsInfo[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        for (auto i = 0u; i < attrCount; ++i) {
            auto const& sourceVertexInfo = desc.vertexInputInfo_->vertexAttributes_[i];
            inputAttributesInfo[i].binding = i;
            inputAttributesInfo[i].location = sourceVertexInfo.location_;
            inputAttributesInfo[i].offset = sourceVertexInfo.offset_;
            inputAttributesInfo[i].format = sourceVertexInfo.format_;
        }

        vertexInputState.pVertexBindingDescriptions = inputBindingsInfo;
        vertexInputState.pVertexAttributeDescriptions = inputAttributesInfo;
    }

    graphicsPipelineInfo.pVertexInputState = &vertexInputState;
    

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    {
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext = nullptr;
        inputAssemblyInfo.flags = VK_FLAGS_NONE;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = false;
    }

    graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


    VkPipelineViewportStateCreateInfo viewportInfo;
    {
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.pNext = nullptr;
        viewportInfo.flags = VK_FLAGS_NONE;

        auto const& viewportCount = desc.viewportInfo_->viewportsCount_;
        viewportInfo.viewportCount = viewportCount;
        for (auto i = 0u; i < viewportCount; ++i) {
            auto const& vDesc = desc.viewportInfo_->viewports_[i];

            auto& v = viewports[i];
            v.x = vDesc.x_;
            v.y = vDesc.y_;
            v.width = vDesc.width_;
            v.height = vDesc.height_;
            v.minDepth = vDesc.minDepth_;
            v.maxDepth = vDesc.maxDepth_;
            
            auto& sc = scissorRects[i];
            sc.offset.x = vDesc.scissorXoffset_;
            sc.offset.y = vDesc.scissorYoffset_;
            sc.extent.width = vDesc.scissorXextent_;
            sc.extent.height = vDesc.scissorYextent_;
        }

        viewportInfo.pViewports = viewports;
        viewportInfo.pScissors = scissorRects;
    }

    graphicsPipelineInfo.pViewportState = &viewportInfo;


    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    {
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.pNext = nullptr;
        rasterizationInfo.flags = VK_FLAGS_NONE;
        //rasterizationInfo.
    }

    graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
    

    // viewport
    // rasterization
    



    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateGraphicsPipelines(device_->Handle(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &vkPipeline));
    return PipelineHandle{ nullptr };
}


}