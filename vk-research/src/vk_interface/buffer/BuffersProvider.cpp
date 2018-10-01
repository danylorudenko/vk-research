#include "BuffersProvider.hpp"

#include <utility>

#include "../Device.hpp"
#include "../ImportTable.hpp"
#include "../resources/ResourcesController.hpp"

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

void BuffersProvider::RegisterViews(std::uint32_t buffersCount, BufferResourceHandle* buffers, BufferViewDesc const* desc, BufferViewHandle* results)
{
    auto const prevSize = bufferViews_.size();
    bufferViews_.resize(prevSize + buffersCount);
    
    VkBufferViewCreateInfo bvInfo;
    bvInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvInfo.pNext = nullptr;
    for (auto i = 0u; i < buffersCount; ++i) {
        BufferResource* resource = resourcesController_->GetBuffer(buffers[i]);

        bvInfo.buffer = resource->handle_;
        bvInfo.flags = VK_FLAGS_NONE;
        bvInfo.format = desc[i].format_;
        bvInfo.offset = desc[i].offset_;
        bvInfo.range = desc[i].size_;

        VkBufferView vkView = VK_NULL_HANDLE;
        VK_ASSERT(table_->vkCreateBufferView(device_->Handle(), &bvInfo, nullptr, &vkView));

        auto& bv = bufferViews_[prevSize + i];
        bv.handle_ = vkView;
        bv.format_ = desc[i].format_;
        bv.offset_ = desc[i].offset_;
        bv.size_ = desc[i].size_;
        bv.resource_ = buffers[i];
        
        results[i].id_ = static_cast<std::uint32_t>(prevSize + i);
    }
}

void BuffersProvider::AcquireBuffers(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferViewHandle* results)
{
    std::uint64_t totalSize = 0;
    VkFormat const format = desc[0].format_;
    BufferUsage const usage = desc[0].usage_;

    for (auto i = 1u; i < buffersCount; ++i) {
        assert(desc[i].usage_ == desc[i - 1].usage_ && "All acquired resources must share same usage pattern");
        totalSize += desc->size_;
    }

    BufferDesc bufferDesc;
    bufferDesc.size_ = totalSize;
    bufferDesc.usage_ = desc[0].usage_;
    BufferResourceHandle bufferRes = resourcesController_->CreateBuffer(bufferDesc);

    auto const prevViewsCount = bufferViews_.size();
    bufferViews_.resize(prevViewsCount + buffersCount);

    VkDevice const device = device_->Handle();

    VkBufferView view = VK_NULL_HANDLE;
    VkBufferViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    for (auto i = 0u; i < buffersCount; ++i) {
        BufferResource* resource = resourcesController_->GetBuffer(bufferRes);
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.buffer = resource->handle_;
        viewInfo.format = format;
        viewInfo.offset = desc[i].offset_;
        viewInfo.range = desc[i].size_;

        VK_ASSERT(table_->vkCreateBufferView(device, &viewInfo, nullptr, &view));

        std::uint32_t viewId = static_cast<std::uint32_t>(prevViewsCount + i);
        auto& resultView = bufferViews_[viewId];
        resultView.handle_ = view;
        resultView.format_ = format;
        resultView.offset_ = desc[i].offset_;
        resultView.size_ = desc[i].size_;
        resultView.resource_ = bufferRes;

        results[i].id_ = viewId;
    }

}

void BuffersProvider::ReleaseBuffers(std::uint32_t buffersCount, BufferViewHandle const* handles)
{

}

}