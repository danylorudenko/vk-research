#pragma once

#include <cstdint>
#include <vulkan\vulkan.h>
#include <limits>

namespace VKW
{

std::uint16_t constexpr MAX_SET_LAYOUT_MEMBERS = 6;
std::uint16_t constexpr MAX_PIPELINE_LAYOUT_MEMBERS = 6;

enum DescriptorStageBits : std::uint32_t
{
    COMPUTE = 1,
    VERTEX = 1 << 1,
    FRAGMENT = 1 << 2,
    RENDERING = VERTEX | FRAGMENT,
    ALL = COMPUTE | RENDERING
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

struct DescriptorSetLayout;

class DescriptorLayoutDecorator
{
    friend class DescriptorLayoutController;
    friend struct DescriptorSetLayout;

public:
    struct DecoratorDescriptorDesc
    {
        DescriptorType  type_;
        std::uint32_t   binding_;
        std::uint32_t   count_              : 16;
        std::uint32_t   variableCount_      : 1;
        std::uint32_t   updateAfterBind_    : 1;
    };


public:
    void Add(DescriptorType type, std::uint16_t count = 1);
    void AddVariableCount(DescriptorType type);

    DescriptorLayoutDecorator& operator=(DescriptorLayoutDecorator const& rhs);

    std::uint16_t GetCount() const;
    DecoratorDescriptorDesc const& GetInfo(std::uint16_t i) const;

private:
    DescriptorLayoutDecorator();

    // these should be called only by DescriptorLayoutController
    void Init(DescriptorSetLayout* owner, DescriptorStage stages);
    void End();

    DescriptorSetLayout* owner_;
    std::uint16_t count_;
    DescriptorStage stagesMask_;
    DecoratorDescriptorDesc info_[MAX_SET_LAYOUT_MEMBERS];
    bool isClosed_;

};

struct DescriptorSetLayout
{    
    VkDescriptorSetLayout handle_ = VK_NULL_HANDLE;
    DescriptorLayoutDecorator info_;

};

struct PipelineLayout;

class PipelineLayoutDecorator
{
    friend class DescriptorLayoutController;
    friend struct PipelineLayout;

public:
    void Add(DescriptorSetLayout* layout);

    std::uint16_t GetCount() const;
    DescriptorSetLayout const& GetLayout(std::uint16_t i) const;
    DescriptorLayoutDecorator const& GetLayoutInfo(std::uint16_t i) const;


private:
    // this should only be called by DescriptorLayoutController
    PipelineLayoutDecorator();
    void Init(PipelineLayout* owner);
    void End();

private:
    PipelineLayout* owner_;
    std::uint16_t count_;
    DescriptorSetLayout* members_[MAX_PIPELINE_LAYOUT_MEMBERS];
    bool isClosed_;
};

struct PipelineLayout
{
    VkPipelineLayout handle_ = VK_NULL_HANDLE;
    PipelineLayoutDecorator info_;

};

}