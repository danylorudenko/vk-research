#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\pipeline\DescriptorLayout.hpp>

#include <vector>
#include <queue>

namespace VKW
{

class Device;
class ImportTable;

enum DescriptorStageBits : std::uint32_t
{
    COMPUTE         = 1,
    VERTEX          = 1 << 1,
    FRAGMENT        = 1 << 2,
    RENDERING       = VERTEX | FRAGMENT,
    ALL             = COMPUTE | RENDERING
};

using DescriptorStage = std::uint32_t;

enum DescriptorType
{
    DESCRIPTOR_TYPE_TEXTURE,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_STORAGE_IMAGE,
    DESCRIPTOR_TYPE_STORAGE_BUFFER
};

struct LayoutDescriptorDesc
{
    DescriptorType  type_;
    std::uint32_t   binding_;
    std::uint32_t   count_    : 16;
    std::uint32_t   unbound_  : 1;
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

public:
    class Decorator
    {
        friend DescriptorLayoutController;

    public:
        void Add(DescriptorType type, std::uint16_t count = 1);
        void AddUnbound(DescriptorType type);
        void End();

        void CopyFrom(Decorator const& other);


        DescriptorStage stagesMask_;
        std::uint16_t count_;
        DescriptorSetLayoutMemberInfo info_[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];

    };

    Decorator*  BeginLayout(DescriptorStage stages);

    DescriptorSetLayout* EndLayout(Decorator* decorator);
    void ReleaseDescriptorSetLayout(DescriptorSetLayout* handle);

    PipelineLayout* EndPendingPipelineLayout();
    void ReleasePipelineLayout(PipelineLayout* handle);

private:
    ImportTable* table_;
    Device* device_;

    bool isLayoutPending_;
    std::queue<Decorator> pendingLayouts_;

    std::vector<DescriptorSetLayout*> setLayouts_;
    std::vector<PipelineLayout*> pipelineLayouts_;
};

}