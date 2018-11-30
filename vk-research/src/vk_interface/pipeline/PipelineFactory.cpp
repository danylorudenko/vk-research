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
    static VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo[RenderPass::MAX_ATTACHMENTS];
    
    PipelineLayoutHandle layoutHandle = descriptorLayoutController_->CreatePipelineLayout(*desc.layoutDesc_);
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
    graphicsPipelineInfo.pStages = shaderStagesInfo;

    static VkPipelineVertexInputStateCreateInfo vertexInputState;
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
    

    static VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    {
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext = nullptr;
        inputAssemblyInfo.flags = VK_FLAGS_NONE;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = false;
    }

    graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


    static VkPipelineViewportStateCreateInfo viewportInfo;
    {
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.pNext = nullptr;
        viewportInfo.flags = VK_FLAGS_NONE;

        auto const& viewportCount = desc.viewportInfo_->viewportsCount_;
        viewportInfo.viewportCount = viewportCount;
        viewportInfo.scissorCount = viewportCount;
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


    static VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    {
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.pNext = nullptr;
        rasterizationInfo.flags = VK_FLAGS_NONE;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationInfo.depthBiasEnable = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor = 0.0f;
        rasterizationInfo.depthBiasClamp = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationInfo.lineWidth = 1.0f;
    }

    graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;



    static VkPipelineMultisampleStateCreateInfo multisampleInfo;
    {
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.pNext = nullptr;
        multisampleInfo.flags = VK_FLAGS_NONE;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE;

    }

    graphicsPipelineInfo.pMultisampleState = &multisampleInfo;



    static VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    {
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.pNext = nullptr;
        colorBlendInfo.flags = VK_FLAGS_NONE;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
        
        auto const attachmentCount = renderPass->colorAttachmentsCount_;
        colorBlendInfo.attachmentCount = attachmentCount;
        for (auto i = 0u; i < attachmentCount; ++i) {
            auto& info = colorBlendAttachmentInfo[i];
            info.blendEnable = VK_FALSE;
            info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            info.colorBlendOp = VK_BLEND_OP_ADD; // don't care
            info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            info.alphaBlendOp = VK_BLEND_OP_ADD; // don't care
            info.colorWriteMask = 
                VK_COLOR_COMPONENT_R_BIT | 
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        }
        colorBlendInfo.pAttachments = colorBlendAttachmentInfo;
    }

    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
    

    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateGraphicsPipelines(device_->Handle(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &vkPipeline));

    auto* result = new Pipeline{};
    result->vkPipeline_ = vkPipeline;

    return PipelineHandle{ result };
}


}