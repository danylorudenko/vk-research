#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\pipeline\DescriptorLayout.hpp>

#include <vector>

namespace VKW
{

class Device;
class ImportTable;

enum DescriptorStage
{
    COMPUTE,
    VERTEX,
    FRAGMENT,
    RENDERING,
    ALL
};

enum DescriptorType
{
    DESCRIPTOR_TYPE_TEXTURE,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_STORAGE_IMAGE
};

struct LayoutDescriptorDesc
{
    DescriptorType type_;
    std::uint32_t binding_;
};

struct DescriptorSetLayoutDesc
{
    DescriptorStage stage_;
    std::uint32_t membersCount_;
    LayoutDescriptorDesc membersDesc_[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};

struct PipelineLayoutDesc
{
    std::uint32_t membersCount_;
    DescriptorSetLayout* members_[PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
};

struct DescriptorLayoutControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class DescriptorLayoutController
    : public NonCopyable
{
public:
    DescriptorLayoutController();
    DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc);

    DescriptorLayoutController(DescriptorLayoutController&& rhs);
    DescriptorLayoutController& operator=(DescriptorLayoutController&& rhs);

    ~DescriptorLayoutController();

    DescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc);
    void ReleaseDescriptorSetLayout(DescriptorSetLayout* handle);

    PipelineLayout* CreatePipelineLayout(PipelineLayoutDesc const& desc);
    void ReleasePipelineLayout(PipelineLayout* handle);

private:
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSetLayout*> setLayouts_;
    std::vector<PipelineLayout*> pipelineLayouts_;
};

}