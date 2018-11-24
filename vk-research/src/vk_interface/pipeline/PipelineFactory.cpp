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

PipelineHandle PipelineFactory::CreateGraphicsPipeline(GraphicsPipelineDesc const& desc)
{
    PipelineLayoutHandle layoutHandle = descriptorLayoutController_->CreatePipelineLayout(desc.layoutDesc_);
    PipelineLayout* layout = descriptorLayoutController_->GetPipelineLayout(layoutHandle);
    


    VkGraphicsPipelineCreateInfo graphicsPipelineInfo;


    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateGraphicsPipelines(device_->Handle(), VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &vkPipeline));
    return PipelineHandle{ nullptr };
}


}