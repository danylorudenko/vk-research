#include "DescriptorLayoutController.hpp"
#include "../Tools.hpp"
#include "../Device.hpp"
#include "../ImportTable.hpp"

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

}

DescriptorSetLayoutHandle DescriptorLayoutController::CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc)
{
    VkDescriptorSetLayoutBinding bindings[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
    for (auto i = 0; i < desc.membersCount_; ++i) {
        bindings[i].pImmutableSamplers = nullptr;
        // WARNING: in the future these flags potentially will require some adjustments
        bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[i].descriptorCount = 1;
        bindings[i].binding = desc.membersDesc_[i].binding_;
        switch (desc.membersDesc_[i].type_)
        {
        case DescriptorType::TEXTURE:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case DescriptorType::UNIFORM_BUFFER:
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
    for (auto i = 0; i < desc.membersCount_; ++i) {
        result->membersInfo_[i].type_ = bindings[i].descriptorType;
        result->membersInfo_[i].binding_ = bindings[i].descriptorType;
    }

    setLayouts_.push_back(result);

    return DescriptorSetLayoutHandle{ result };
}

void DescriptorLayoutController::ReleaseDescriptorSetLayout(DescriptorSetLayoutHandle handle)
{
    auto setLayoutIt = std::find(setLayouts_.begin(), setLayouts_.end(), handle.layout_);
    if (setLayoutIt == setLayouts_.end()) {
        assert(false && "Attempt to release invalid DescriptorSetLayoutHandle.");
    }

    DescriptorSetLayout* setLayout = *setLayoutIt;
    table_->vkDestroyDescriptorSetLayout(device_->Handle(), setLayout->handle_, nullptr);

    delete setLayout;
    setLayouts_.erase(setLayoutIt);
}

}