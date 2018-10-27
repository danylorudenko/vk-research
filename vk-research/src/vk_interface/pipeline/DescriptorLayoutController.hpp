#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "DescriptorLayout.hpp"

#include <vector>

namespace VKW
{

class Device;
class ImportTable;

enum class DescriptorType
{
    TEXTURE,
    UNIFORM_BUFFER
};

struct DescriptorDesc
{
    DescriptorType type_;
    std::uint32_t binding_;
};

struct DescriptorSetLayoutDesc
{
    std::uint32_t membersCount_;
    DescriptorDesc membersDesc_[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};

struct PipelineLayoutDesc
{
    std::uint32_t membersCount_;
    DescriptorSetLayoutHandle members_[PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
};

struct DescriptorLayoutControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class DescriptorLayoutController
    : public NonCopyable
{
    DescriptorLayoutController();
    DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc);

    DescriptorLayoutController(DescriptorLayoutController&& rhs);
    DescriptorLayoutController& operator=(DescriptorLayoutController&& rhs);

    ~DescriptorLayoutController();

    DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc);
    void ReleaseDescriptorSetLayout(DescriptorSetLayoutHandle handle);

    PipelineLayoutHandle CreatePipelineLayout(PipelineLayoutDesc const& desc);
    void ReleasePipelineLayout(PipelineLayoutHandle handle);
    
private:
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSetLayout*> setLayouts_;
    std::vector<PipelineLayout*> pipelineLayouts_;
};

}