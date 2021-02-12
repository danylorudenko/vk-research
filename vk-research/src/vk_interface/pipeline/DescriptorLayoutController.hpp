#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\pipeline\DescriptorLayout.hpp>

#include <vector>
#include <queue>

namespace VKW
{

class Device;
class ImportTable;

struct DescriptorLayoutControllerDesc
{
    ImportTable* table_;
    Device* device_;
};

class DescriptorLayoutController
    : public NonCopyable
{
public:

    friend DescriptorLayoutDecorator;
    friend PipelineLayoutDecorator;

    DescriptorLayoutController();
    DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc);

    DescriptorLayoutController(DescriptorLayoutController&& rhs);
    DescriptorLayoutController& operator=(DescriptorLayoutController&& rhs);

    ~DescriptorLayoutController();

public:
    DescriptorLayoutDecorator*  BeginLayout(DescriptorStage stages);
    DescriptorSetLayout*        EndLayout(DescriptorLayoutDecorator* decorator);

    PipelineLayoutDecorator*    BeginPipelineLayout();
    PipelineLayout*             EndPipelineLayout(PipelineLayoutDecorator* decorator);

    void                        ReleaseLayout(DescriptorSetLayout* handle);
    void                        ReleaseLayout(PipelineLayout* handle);

private:
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSetLayout*> setLayouts_;
    std::vector<PipelineLayout*> pipelineLayouts_;
};

}