#include "BuffersProvider.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\resources\ResourcesController.hpp>

namespace VKW
{

BuffersProvider::BuffersProvider()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{

}

BuffersProvider::BuffersProvider(BuffersProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ desc.resourcesController_ }
{

}

BuffersProvider::BuffersProvider(BuffersProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

BuffersProvider& BuffersProvider::operator=(BuffersProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);

    std::swap(bufferViews_, rhs.bufferViews_);

    return *this;
}

void BuffersProvider::CreateViewsAndCreateBuffers(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferView** results)
{
    std::uint32_t totalBufferSize = desc[0].size_;
    VkFormat const format = desc[0].format_;
    BufferUsage const usage = desc[0].usage_;

    for (auto i = 1u; i < buffersCount; ++i) {
        assert(desc[i].usage_ == desc[i - 1].usage_ && "All acquired resources must share same usage pattern");
        totalBufferSize += desc[i].size_;
    }

    BufferDesc bufferDesc;
    bufferDesc.size_ = totalBufferSize;
    bufferDesc.usage_ = desc[0].usage_;
    BufferResource* bufferRes = resourcesController_->CreateBuffer(bufferDesc);
    std::uint32_t* referenceCounter = new std::uint32_t{ buffersCount };


    VkDevice const device = device_->Handle();

    VkBufferView view = VK_NULL_HANDLE;
    VkBufferViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    std::uint32_t prevOffset = 0;
    for (auto i = 0u; i < buffersCount; ++i) {
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.buffer = bufferRes->handle_;
        viewInfo.format = format;
        //viewInfo.offset = desc[i].offset_;
        viewInfo.offset = prevOffset;
        prevOffset += desc[i].size_;
        viewInfo.range = desc[i].size_;

        if (format != VK_FORMAT_UNDEFINED) {
            VK_ASSERT(table_->vkCreateBufferView(device, &viewInfo, nullptr, &view));
        }
        
        BufferView* resultView = new BufferView{ view, format, viewInfo.offset, viewInfo.range, bufferRes, referenceCounter };
        bufferViews_.emplace(resultView);
        
        results[i] = resultView;
    }

}

void BuffersProvider::ReleaseViewsAndBuffers(std::uint32_t buffersCount, BufferView** views)
{
    VkDevice const device = device_->Handle();

    for (auto i = 0u; i < buffersCount; ++i) {
        BufferView* view = views[i];

        auto result = bufferViews_.erase(view);
        assert(result > 0 && "Failed to release BufferView");

        if (view->handle_ != VK_NULL_HANDLE) {
            table_->vkDestroyBufferView(device, view->handle_, nullptr);
        }
        
        if (--view->bufferResourceReferenceCount_ == 0)
        {
            resourcesController_->FreeBuffer(view->bufferResource_);
        }

        delete view;
        views[i] = nullptr;
    }

}

BuffersProvider::~BuffersProvider()
{
    std::vector<BufferView*> viewsToRelease{ bufferViews_.begin(), bufferViews_.end() };

    ReleaseViewsAndBuffers(static_cast<std::uint32_t>(viewsToRelease.size()), viewsToRelease.data());
}

}