#include "DescriptorLayoutController.hpp"

#include <vk_interface\Tools.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>

#include <utility>
#include <cassert>
#include <algorithm>

namespace VKW
{

DescriptorLayoutDecorator::DescriptorLayoutDecorator()
    : owner_{ nullptr }
    , stagesMask_{ 0 }
    , count_{ 0 }
    , isClosed_{ false }
{}

void DescriptorLayoutDecorator::Add(DescriptorType type, std::uint16_t count)
{
    assert(!isClosed_ && "Attempt to modify closed descriptor layout decorator.");
    assert((count_ + 1 < MAX_SET_LAYOUT_MEMBERS) && "Maximum count of descriptor bindings in set is MAX_SET_LAYOUT_MEMBERS");
    
    info_[count_].type_ = type;
    info_[count_].binding_ = count_;
    info_[count_].count_ = count;
    info_[count_].variableCount_ = 0;
    info_[count_].updateAfterBind_ = 0;
    count_++;
}

void DescriptorLayoutDecorator::AddVariableCount(DescriptorType type)
{
    assert(!isClosed_ && "Attempt to modify closed descriptor layout decorator.");
    assert((count_ + 1 < MAX_SET_LAYOUT_MEMBERS) && "Maximum count of descriptor bindings in set is MAX_SET_LAYOUT_MEMBERS");

    info_[count_].type_ = type;
    info_[count_].binding_ = count_;
    info_[count_].count_ = 10000;
    info_[count_].variableCount_ = 1;
    info_[count_].updateAfterBind_ = 1;
    count_++;

    isClosed_ = true;
}

void DescriptorLayoutDecorator::Init(DescriptorSetLayout* owner, DescriptorStage stages)
{
    owner_ = owner;
    stagesMask_ = stages;
}

void DescriptorLayoutDecorator::End()
{
    assert(!isClosed_ && "Attempt to close descriptor layout that is already closed.");
    isClosed_ = true;
}

DescriptorLayoutDecorator& DescriptorLayoutDecorator::operator=(DescriptorLayoutDecorator const&) = default;

std::uint16_t DescriptorLayoutDecorator::GetCount() const
{
    return count_;
}

DescriptorLayoutDecorator::DecoratorDescriptorDesc const& DescriptorLayoutDecorator::GetInfo(std::uint16_t i) const
{
    assert(i < count_);
    return info_[i];
}

PipelineLayoutDecorator::PipelineLayoutDecorator()
    : owner_{ nullptr }
    , count_{ 0 }
    , isClosed_{ nullptr }
{}

void PipelineLayoutDecorator::Init(PipelineLayout* owner)
{
    owner_ = owner;
}

void PipelineLayoutDecorator::Add(DescriptorSetLayout* layout)
{
    assert(!isClosed_ && "Attempt to modify closed pipeline layout decorator.");
    assert(count_ + 1 < MAX_PIPELINE_LAYOUT_MEMBERS && "Maximum count of descriptor sets in pipeline layout is MAX_PIPELINE_LAYOUT_MEMBERS.");
    members_[count_++] = layout;
}

std::uint16_t PipelineLayoutDecorator::GetCount() const
{
    return count_;
}

DescriptorSetLayout const& PipelineLayoutDecorator::GetLayout(std::uint16_t i) const
{
    assert(i < count_);
    return *members_[i];
}

void PipelineLayoutDecorator::End()
{
    assert(!isClosed_ && "Attempt to close already closed pipeline layout.");
    isClosed_ = true;
}



DescriptorLayoutController::DescriptorLayoutController()
    : table_{ nullptr }
    , device_{ nullptr }
{

}

DescriptorLayoutController::DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{
}

DescriptorLayoutController::DescriptorLayoutController(DescriptorLayoutController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

DescriptorLayoutController& DescriptorLayoutController::operator=(DescriptorLayoutController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    std::swap(setLayouts_, rhs.setLayouts_);
    std::swap(pipelineLayouts_, rhs.pipelineLayouts_);

    return *this;
}

DescriptorLayoutController::~DescriptorLayoutController()
{
    VkDevice const device = device_->Handle();

    for (auto const& pipelineLayout : pipelineLayouts_) {
        table_->vkDestroyPipelineLayout(device, pipelineLayout->handle_, nullptr);
        delete pipelineLayout;
    }

    for (auto const& setLayout : setLayouts_) {
        table_->vkDestroyDescriptorSetLayout(device, setLayout->handle_, nullptr);
        delete setLayout;
    }

}

DescriptorLayoutDecorator* DescriptorLayoutController::BeginLayout(DescriptorStage stages)
{
    DescriptorSetLayout* layout = new DescriptorSetLayout{};
    layout->info_.Init(layout, stages);
    return &layout->info_;
}

VkDescriptorType DescriptorTypeToVK(DescriptorType type)
{
    switch (type)
    {
    case DescriptorType::DESCRIPTOR_TYPE_TEXTURE:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    case DescriptorType::DESCRIPTOR_TYPE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;

    case DescriptorType::DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    case DescriptorType::DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    case DescriptorType::DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    default:
        assert(false && "Unsupported DescriptorType.");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

DescriptorSetLayout* DescriptorLayoutController::EndLayout(DescriptorLayoutDecorator* decorator)
{
    decorator->End();

    VkFlags stageFlags = VK_FLAGS_NONE;

    if(decorator->stagesMask_ & DescriptorStageBits::COMPUTE)
        stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;

    if (decorator->stagesMask_ & DescriptorStageBits::VERTEX)
        stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;

    if (decorator->stagesMask_ & DescriptorStageBits::FRAGMENT)
        stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    
    static VkDescriptorSetLayoutBinding bindings[MAX_SET_LAYOUT_MEMBERS];
    static VkDescriptorBindingFlags bindingFlags[MAX_SET_LAYOUT_MEMBERS];
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsInfo.pNext = nullptr;
    bindingFlagsInfo.bindingCount = decorator->count_;
    bindingFlagsInfo.pBindingFlags = bindingFlags;

    for (std::uint32_t i = 0u; i < decorator->count_; ++i) {
        bindings[i].binding = decorator->info_[i].binding_;
        bindings[i].descriptorType = DescriptorTypeToVK(decorator->info_[i].type_);
        bindings[i].descriptorCount = decorator->count_;
        bindings[i].stageFlags = stageFlags;
        bindings[i].pImmutableSamplers = nullptr;

        bindingFlags[i] = VK_FLAGS_NONE;
        if (decorator->info_[i].variableCount_)
            bindingFlags[i] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        if(decorator->info_[i].updateAfterBind_)
            bindingFlags[i] |= 
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | 
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | 
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    }

    VkDescriptorSetLayoutCreateInfo setInfo;
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pNext = &bindingFlagsInfo;
    setInfo.bindingCount = decorator->count_;
    setInfo.pBindings = bindings;
    setInfo.flags = VK_FLAGS_NONE;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateDescriptorSetLayout(device_->Handle(), &setInfo, nullptr, &setLayout));

    DescriptorSetLayout* result = decorator->owner_;
    result->handle_ = setLayout;
    
    setLayouts_.push_back(result);

    return result;
}

void DescriptorLayoutController::ReleaseLayout(DescriptorSetLayout* handle)
{
    auto setLayoutIt = std::find(setLayouts_.begin(), setLayouts_.end(), handle);
    if (setLayoutIt == setLayouts_.end()) {
        assert(false && "Attempt to release invalid DescriptorSetLayoutHandle.");
    }

    DescriptorSetLayout* setLayout = *setLayoutIt;
    table_->vkDestroyDescriptorSetLayout(device_->Handle(), setLayout->handle_, nullptr);

    delete setLayout;
    setLayouts_.erase(setLayoutIt);
}

PipelineLayoutDecorator* DescriptorLayoutController::BeginPipelineLayout()
{
    PipelineLayout* layout = new PipelineLayout{};
    layout->info_.Init(layout);
    return &layout->info_;
}

PipelineLayout* DescriptorLayoutController::EndPipelineLayout(PipelineLayoutDecorator* decorator)
{
    VkDescriptorSetLayout layouts[MAX_PIPELINE_LAYOUT_MEMBERS];
    std::uint16_t const count = decorator->count_;

    for (std::uint16_t i = 0u; i < count; ++i) {
        DescriptorSetLayout* layout = decorator->members_[i];
        layouts[i] = layout->handle_;
    }


    VkPipelineLayoutCreateInfo cInfo;
    cInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    cInfo.pNext = nullptr;
    cInfo.flags = VK_FLAGS_NONE;
    cInfo.pushConstantRangeCount = 0;
    cInfo.pPushConstantRanges = nullptr;
    cInfo.setLayoutCount = count;
    cInfo.pSetLayouts = layouts;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreatePipelineLayout(device_->Handle(), &cInfo, nullptr, &pipelineLayout));

    PipelineLayout* result = decorator->owner_;
    result->handle_ = pipelineLayout;

    pipelineLayouts_.push_back(result);

    return result;
}

void DescriptorLayoutController::ReleaseLayout(PipelineLayout* handle)
{
    auto layoutIt = std::find(pipelineLayouts_.begin(), pipelineLayouts_.end(), handle);
    if (layoutIt == pipelineLayouts_.end()) {
        assert(false && "Attempt to release invalid PipelineLayout.");
    }

    PipelineLayout* layout = *layoutIt;
    table_->vkDestroyPipelineLayout(device_->Handle(), layout->handle_, nullptr);

    delete layout;
    pipelineLayouts_.erase(layoutIt);
}

}