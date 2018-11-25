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
    static VkVertexInputBindingDescription inputBindigsInfo[Pipeline::MAX_VERTEX_ATTRIBUTES];
    static VkVertexInputAttributeDescription inputAttributesInfo[Pipeline::MAX_VERTEX_ATTRIBUTES];
    
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
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.pNext = nullptr;
    //vertexInputState.
    


    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateGraphicsPipelines(device_->Handle(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &vkPipeline));
    return PipelineHandle{ nullptr };
}


}