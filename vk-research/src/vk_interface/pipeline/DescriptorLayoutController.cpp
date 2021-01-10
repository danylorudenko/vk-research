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
    , isLayoutPending_{ false }
{

}

DescriptorLayoutController::DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , isLayoutPending_{ false }
{
      VkDescriptorSetLayoutBinding binding;
      binding.binding = 0;
      binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      binding.descriptorCount = 10 * 1000;
      binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      binding.pImmutableSamplers = nullptr;

      VkDescriptorBindingFlags flag = 
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |         // can write descriptor after bound
          VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | // unknown size, must be last in set
          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |           // not all descriptors must be valid on the whole set (unless they're used)
          VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;// can update descriptors in set that is in execution (unless that particular descriptor is used right now)

      VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags;
      bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
      bindingFlags.pNext = nullptr;
      bindingFlags.bindingCount = 1;
      bindingFlags.pBindingFlags = &flag;

      VkDescriptorSetLayoutCreateInfo createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = &bindingFlags;
      createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
      createInfo.bindingCount = 1;
      createInfo.pBindings = &binding;

      VkDescriptorSetLayout layoutHandle = VK_NULL_HANDLE;
      VK_ASSERT(table_->vkCreateDescriptorSetLayout(device_->Handle(), &createInfo, nullptr, &layoutHandle));
      
      std::cout << "hello" << std::endl;

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

DescriptorSetLayout* DescriptorLayoutController::EndLayout(Decorator const* desc)
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
    VK_ASSERT(table_->vkCreateDescriptorSetLayout(device_->Handle(), &setInfo, nullptr, &setLayout));

    DescriptorSetLayout* result = new DescriptorSetLayout{};
    result->handle_ = setLayout;
    result->membersCount_ = desc.membersCount_;
    for (auto i = 0u; i < desc.membersCount_; ++i) {
        result->membersInfo_[i].type_ = bindings[i].descriptorType;
        result->membersInfo_[i].binding_ = bindings[i].binding;
    }

    setLayouts_.push_back(result);

    return result;
}

void DescriptorLayoutController::ReleaseDescriptorSetLayout(DescriptorSetLayout* handle)
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
    VK_ASSERT(table_->vkCreatePipelineLayout(device_->Handle(), &cInfo, nullptr, &pipelineLayout));

    auto* result = new PipelineLayout{};
    result->handle_ = pipelineLayout;
    result->membersCount_ = desc.membersCount_;
    for (auto i = 0u; i < desc.membersCount_; ++i) {
        result->setLayoutMembers_[i] = desc.members_[i];
    }

    pipelineLayouts_.push_back(result);

    return result;
}

void DescriptorLayoutController::ReleasePipelineLayout(PipelineLayout* handle)
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