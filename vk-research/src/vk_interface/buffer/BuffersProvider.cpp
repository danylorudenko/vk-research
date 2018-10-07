#include "BuffersProvider.hpp"
#include "ProvidedBuffer.hpp"

#include <utility>
#include <algorithm>

#include "../Device.hpp"
#include "../ImportTable.hpp"
#include "../resources/ResourcesController.hpp"

namespace VKW
{

ProvidedBuffer::ProvidedBuffer(BufferResourceHandle resource, std::uint32_t referenceCount)
    : bufferResource_{ resource }
    , referenceCount_{ referenceCount }
{

}

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
    std::swap(providedBuffers_, rhs.providedBuffers_);

    return *this;
}

void BuffersProvider::AcquireViews(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferViewHandle* results)
{
    std::uint64_t totalBufferSize = 0;
    VkFormat const format = desc[0].format_;
    BufferUsage const usage = desc[0].usage_;

    for (auto i = 1u; i < buffersCount; ++i) {
        assert(desc[i].usage_ == desc[i - 1].usage_ && "All acquired resources must share same usage pattern");
        totalBufferSize += desc->size_;
    }

    BufferDesc bufferDesc;
    bufferDesc.size_ = totalBufferSize;
    bufferDesc.usage_ = desc[0].usage_;
    BufferResourceHandle bufferRes = resourcesController_->CreateBuffer(bufferDesc);
    auto* providedBuffer = new ProvidedBuffer{ bufferRes, buffersCount };
    providedBuffers_.push_back(providedBuffer);


    VkDevice const device = device_->Handle();

    VkBufferView view = VK_NULL_HANDLE;
    VkBufferViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    for (auto i = 0u; i < buffersCount; ++i) {
        BufferResource const* resource = resourcesController_->GetBuffer(bufferRes);
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.buffer = resource->handle_;
        viewInfo.format = format;
        viewInfo.offset = desc[i].offset_;
        viewInfo.range = desc[i].size_;

        VK_ASSERT(table_->vkCreateBufferView(device, &viewInfo, nullptr, &view));

        BufferView* resultView = new BufferView{ view, format, desc[i].offset_, desc[i].size_, providedBuffer };
        bufferViews_.push_back(resultView);

        results[i].view_ = resultView;
    }

}

void BuffersProvider::ReleaseViews(std::uint32_t buffersCount, BufferViewHandle const* handles)
{
    VkDevice const device = device_->Handle();

    for (auto i = 0u; i < buffersCount; ++i) {
        auto viewIt = std::find(bufferViews_.cbegin(), bufferViews_.cend(), handles[i].view_);
        assert(viewIt != bufferViews_.cend() && "Can't find BufferView to release.");

        auto providedBufferIt = std::find(providedBuffers_.cbegin(), providedBuffers_.cend(), handles[i].view_->providedBuffer_);
        assert(providedBufferIt != providedBuffers_.cend() && "Can't find any buffer attached to the view.");

        BufferView const& view = **viewIt; // pointer in iterator
        ProvidedBuffer& providedBuffer = **providedBufferIt; // same

        table_->vkDestroyBufferView(device, view.handle_, nullptr);

        if (--providedBuffer.referenceCount_ == 0) {
            resourcesController_->FreeBuffer(providedBuffer.bufferResource_);
            providedBuffers_.erase(providedBufferIt);
        }

        bufferViews_.erase(viewIt);
    }

}

BufferView& BuffersProvider::GetView(BufferViewHandle handle)
{
    return *handle.view_;
}

}