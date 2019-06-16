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

// 8 in spec without wierd extensions. don't think i gonna need more
std::uint32_t constexpr PIPELINE_DYNAMIC_STATES_MAX = 8;

enum PipelineDynamicStatesFlags
{
    PIPELINE_DYNAMIC_STATE_NONE         = 0,
    PIPELINE_DYNAMIC_STATE_VIEWPORT     = 1 << 0,
    PIPELINE_DYNAMIC_STATE_SCISSOR      = 1 << 1,
};

enum PipelineBlendingState
{
    PIPELINE_BLENDING_NONE              = 0,
    PIPELINE_BLENDING_ADDITIVE          = 1 << 0,
    PIPELINE_BLENDING_SRC_ALPHA_DST_ONE = 1 << 1,
    PIPELINE_BLENDING_SRC_ONE_DST_ALPHA = 1 << 2
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
    std::uint32_t dynamicStatesFlags_; // PipelineDynamicStateFlags
    std::uint32_t blendingState_; // PipelineBlendingState
    
    PipelineLayoutDesc* layoutDesc_;
    RenderPassHandle renderPass_;
};

struct ComputePipelineDesc
{
    bool optimized_;
    ShaderStageInfo shaderStage_;
    PipelineLayoutDesc* layoutDesc_;
};

class PipelineFactory
{

public:
    PipelineFactory();
    PipelineFactory(PipelineFactoryDesc const& desc);

    PipelineFactory(PipelineFactory&& rhs);
    PipelineFactory& operator=(PipelineFactory&& rhs);

    PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc const& desc);
    PipelineHandle CreateComputePipeline(ComputePipelineDesc const& desc);
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