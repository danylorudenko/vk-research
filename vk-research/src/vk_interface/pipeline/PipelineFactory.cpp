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
    VkDevice const device = device_->Handle();
    for (auto const& pipeline : pipelines_) {
        table_->vkDestroyPipeline(device, pipeline->vkPipeline_, nullptr);
        delete pipeline;
    }
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

        std::uint32_t const attrCount = desc.vertexInputInfo_->vertexAttributesCount_;

        vertexInputState.vertexBindingDescriptionCount = attrCount > 0 ? 1 : 0;
        vertexInputState.vertexAttributeDescriptionCount = attrCount;

        inputBindingsInfo[0].binding = desc.vertexInputInfo_->binding_;
        inputBindingsInfo[0].stride = desc.vertexInputInfo_->stride_;
        inputBindingsInfo[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        for (auto i = 0u; i < attrCount; ++i) {
            VKW::VertexInputInfo::Attribute const& sourceVertexInfo = desc.vertexInputInfo_->vertexAttributes_[i];
            inputAttributesInfo[i].binding = desc.vertexInputInfo_->binding_;
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
        // TODO
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        //rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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



    static VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    {
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.pNext = nullptr;
        depthStencilInfo.flags = VK_FLAGS_NONE;
        depthStencilInfo.depthTestEnable = desc.depthStencilInfo_->depthTestEnabled_ ? VK_TRUE : VK_FALSE;
        depthStencilInfo.depthWriteEnable = desc.depthStencilInfo_->depthWriteEnabled_ ? VK_TRUE : VK_FALSE;
        depthStencilInfo.depthCompareOp = desc.depthStencilInfo_->depthCompareOp_;
        depthStencilInfo.depthBoundsTestEnable = false;
        depthStencilInfo.stencilTestEnable = desc.depthStencilInfo_->stencilTestEnabled_ ? VK_TRUE : VK_FALSE;
        depthStencilInfo.front = desc.depthStencilInfo_->frontStencilState_;
        depthStencilInfo.back = desc.depthStencilInfo_->backStencilState_;
        depthStencilInfo.minDepthBounds = 0.0f;
        depthStencilInfo.maxDepthBounds = 0.0f;
    }

    graphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;


    static VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    {
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.pNext = nullptr;
        colorBlendInfo.flags = VK_FLAGS_NONE;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
        
        std::uint32_t const attachmentCount = renderPass->colorAttachmentsCount_;

        VkPipelineColorBlendAttachmentState primerState;
        switch (desc.blendingState_) {
        case PIPELINE_BLENDING_ADDITIVE:
            primerState.blendEnable = VK_TRUE;
            primerState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            primerState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.colorBlendOp = VK_BLEND_OP_ADD;
            primerState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.alphaBlendOp = VK_BLEND_OP_MAX;
            break;
        case PIPELINE_BLENDING_SRC_ALPHA_DST_ONE:
            primerState.blendEnable = VK_TRUE;
            primerState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            primerState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.colorBlendOp = VK_BLEND_OP_ADD;
            primerState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.alphaBlendOp = VK_BLEND_OP_MAX;
            break;
        case PIPELINE_BLENDING_SRC_ONE_DST_ALPHA:
            primerState.blendEnable = VK_TRUE;
            primerState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            primerState.colorBlendOp = VK_BLEND_OP_ADD;
            primerState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.alphaBlendOp = VK_BLEND_OP_MAX;
            break;

        default:
            primerState.blendEnable = VK_FALSE;
            primerState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.colorBlendOp = VK_BLEND_OP_ADD; // don't care
            primerState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            primerState.alphaBlendOp = VK_BLEND_OP_ADD; // don't care
        }

        primerState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        colorBlendInfo.attachmentCount = attachmentCount;
        for (std::uint32_t i = 0u; i < attachmentCount; ++i) {
            VkPipelineColorBlendAttachmentState& blendState = colorBlendAttachmentInfo[i];
            blendState = primerState;
        }
        colorBlendInfo.pAttachments = colorBlendAttachmentInfo;
    }

    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;


    static VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    static VkDynamicState dynamicStates[PIPELINE_DYNAMIC_STATES_MAX];
    {
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pNext = nullptr;
        dynamicStateInfo.flags = VK_FLAGS_NONE;
        dynamicStateInfo.dynamicStateCount = 0;
        dynamicStateInfo.pDynamicStates = dynamicStates;

        if (desc.dynamicStatesFlags_ & PIPELINE_DYNAMIC_STATE_VIEWPORT)
            dynamicStates[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
        if (desc.dynamicStatesFlags_ & PIPELINE_DYNAMIC_STATE_SCISSOR)
            dynamicStates[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    }

    graphicsPipelineInfo.pDynamicState = dynamicStateInfo.dynamicStateCount > 0 ? &dynamicStateInfo : nullptr;
    

    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateGraphicsPipelines(device_->Handle(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &vkPipeline));

    auto* result = new Pipeline{};
    result->vkPipeline_ = vkPipeline;
    result->layoutHandle = layoutHandle;

    pipelines_.emplace_back(result);

    return PipelineHandle{ result };
}

Pipeline* PipelineFactory::GetPipeline(PipelineHandle handle) const
{
    return handle.pipeline_;
}

}