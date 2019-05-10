#pragma once

#include "..\..\class_features\NonCopyable.hpp"

#include <vulkan\vulkan.h>
#include <vector>

#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "ShaderModule.hpp"
#include "DescriptorLayoutController.hpp"

namespace VKW
{

class ImportTable;
class Device;
class RenderPassController;
class ShaderModuleFactory;
class DescriptorLayoutController;

struct PipelineFactoryDesc
{
    ImportTable* table_;
    Device* device_;
    RenderPassController* renderPassController_;
    ShaderModuleFactory* shaderModuleFactory_;
    DescriptorLayoutController* descriptorLayoutController_;
};

struct ShaderStageInfo
{
    ShaderModuleHandle shaderModuleHandle_;
};

struct VertexInputInfo
{
    std::uint32_t binding_;
    std::uint32_t stride_;
    std::uint32_t vertexAttributesCount_;
    struct Attribute {
        std::uint32_t location_;
        std::uint32_t offset_;
        VkFormat format_;
    } vertexAttributes_[Pipeline::MAX_VERTEX_ATTRIBUTES];
};

struct InputAssemblyInfo
{
    VkPrimitiveTopology primitiveTopology_;
    bool primitiveRestartEnable_;
};

struct ViewportInfo
{
    std::uint32_t viewportsCount_;
    struct Viewport {
        float x_;
        float y_;
        float width_;
        float height_;
        float minDepth_;
        float maxDepth_;
        
        std::int32_t scissorXoffset_;
        std::int32_t scissorYoffset_;
        std::uint32_t scissorXextent_;
        std::uint32_t scissorYextent_;
    } 
    viewports_[Pipeline::MAX_VIEWPORTS];
};

struct DepthStencilInfo
{
    bool depthTestEnabled_;
    bool depthWriteEnabled_;
    VkCompareOp depthCompareOp_;

    bool stencilTestEnabled_;
    VkStencilOpState frontStencilState_;
    VkStencilOpState backStencilState_;
};

struct GraphicsPipelineDesc
{
    bool optimized_;

    std::uint32_t shaderStagesCount_;
    ShaderStageInfo shaderStages_[Pipeline::MAX_SHADER_STAGES];

    InputAssemblyInfo* inputAssemblyInfo_;
    VertexInputInfo* vertexInputInfo_;
    ViewportInfo* viewportInfo_;
    DepthStencilInfo* depthStencilInfo_;
    // blending info should be here later
    
    PipelineLayoutDesc* layoutDesc_;
    RenderPassHandle renderPass_;
};

class PipelineFactory
{

public:
    PipelineFactory();
    PipelineFactory(PipelineFactoryDesc const& desc);

    PipelineFactory(PipelineFactory&& rhs);
    PipelineFactory& operator=(PipelineFactory&& rhs);

    PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc const& desc);
    void DestroyPipeline(PipelineHandle pipeline);
    
    Pipeline* GetPipeline(PipelineHandle handle) const;
    
    ~PipelineFactory();

private:
    ImportTable* table_;
    Device* device_;
    RenderPassController* renderPassController_;
    ShaderModuleFactory* shaderModuleFactory_;
    DescriptorLayoutController* descriptorLayoutController_;


    std::vector<Pipeline*> pipelines_;
};

}