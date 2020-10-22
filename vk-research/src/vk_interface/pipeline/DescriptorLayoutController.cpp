#include "DescriptorLayoutController.hpp"

#include <vk_interface\Tools.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>

#include <utility>
#include <cassert>
#include <algorithm>

namespace VKW
{

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

    for (PipelineLayout* pipelineLayout : pipelineLayouts_) {
        table_->vkDestroyPipelineLayout(device, pipelineLayout->handle_, nullptr);
        delete pipelineLayout;
    }

    for (DescriptorSetLayout* setLayout : setLayouts_) {
        table_->vkDestroyDescriptorSetLayout(device, setLayout->handle_, nullptr);
        delete setLayout;
    }

}

DescriptorSetLayout* DescriptorLayoutController::CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc)
{
    VkFlags stageFlags = VK_FLAGS_NONE;
    switch (desc.stage_)
    {
    case DescriptorStage::COMPUTE:
        stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    case DescriptorStage::VERTEX:
        stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case DescriptorStage::FRAGMENT:
        stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case DescriptorStage::RENDERING:
        stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case DescriptorStage::ALL:
        stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    default:
        assert(false && "Descriptor stage for DescriptorSetLayout is not supported!");
    }

    VkDescriptorSetLayoutBinding bindings[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    for (std::uint32_t i = 0u; i < desc.membersCount_; ++i) {
        bindings[i].pImmutableSamplers = nullptr;
        // TODO: in the future these flags potentially will require some adjustments
        bindings[i].stageFlags = stageFlags;
        bindings[i].descriptorCount = 1;
        bindings[i].binding = desc.membersDesc_[i].binding_;
        switch (desc.membersDesc_[i].type_)
        {
        case DescriptorType::DESCRIPTOR_TYPE_TEXTURE:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
            break;
        case DescriptorType::DESCRIPTOR_TYPE_SAMPLER:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            break;
        case DescriptorType::DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case DescriptorType::DESCRIPTOR_TYPE_STORAGE_IMAGE:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        default:
            assert(false && "Unsupported DescriptorType.");
        }

    }
    
    VkDescriptorSetLayoutCreateInfo setInfo;
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pNext = nullptr;
    setInfo.bindingCount = desc.membersCount_;
    setInfo.pBindings = bindings;
    setInfo.flags = VK_FLAGS_NONE;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    ERR_GUARD_VK(table_->vkCreateDescriptorSetLayout(device_->Handle(), &setInfo, nullptr, &setLayout));

    DescriptorSetLayout* result = new DescriptorSetLayout{};
    result->handle_ = setLayout;
    result->membersCount_ = desc.membersCount_;
    for (auto i = 0u; i < desc.membersCount_; ++i) {
        result->membersInfo_[i].type_ = bindings[i].descriptorType;
        result->membersInfo_[i].binding_ = bindings[i].binding;
    }

    setLayouts_.emplace(result);

    return result;
}

void DescriptorLayoutController::ReleaseDescriptorSetLayout(DescriptorSetLayout* layout)
{
    auto setLayoutIt = std::find(setLayouts_.begin(), setLayouts_.end(), layout);
    if (setLayoutIt == setLayouts_.end()) {
        assert(false && "Attempt to release invalid DescriptorSetLayoutHandle.");
    }

    DescriptorSetLayout* setLayout = *setLayoutIt;
    table_->vkDestroyDescriptorSetLayout(device_->Handle(), setLayout->handle_, nullptr);

    delete setLayout;
    setLayouts_.erase(setLayoutIt);
}

PipelineLayout* DescriptorLayoutController::CreatePipelineLayout(PipelineLayoutDesc const& desc)
{
    VkDescriptorSetLayout layouts[PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    for (auto i = 0u; i < desc.membersCount_; ++i) {
        DescriptorSetLayout* layout = desc.members_[i];
        layouts[i] = layout->handle_;
    }


    VkPipelineLayoutCreateInfo cInfo;
    cInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    cInfo.pNext = nullptr;
    cInfo.flags = VK_FLAGS_NONE;
    cInfo.pushConstantRangeCount = 0;
    cInfo.pPushConstantRanges = nullptr;
    cInfo.setLayoutCount = desc.membersCount_;
    cInfo.pSetLayouts = layouts;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    ERR_GUARD_VK(table_->vkCreatePipelineLayout(device_->Handle(), &cInfo, nullptr, &pipelineLayout));

    auto* result = new PipelineLayout{};
    result->handle_ = pipelineLayout;
    result->membersCount_ = desc.membersCount_;
    for (auto i = 0u; i < desc.membersCount_; ++i) {
        result->setLayoutMembers_[i] = desc.members_[i];
    }

    pipelineLayouts_.emplace(result);

    return result;
}

void DescriptorLayoutController::ReleasePipelineLayout(PipelineLayout* layout)
{
    assert((pipelineLayouts_.find(layout) != pipelineLayouts_.end()) && "Attempt to release invalid PipelineLayout.");

    table_->vkDestroyPipelineLayout(device_->Handle(), layout->handle_, nullptr);

    delete layout;
    pipelineLayouts_.erase(layout);
}

}