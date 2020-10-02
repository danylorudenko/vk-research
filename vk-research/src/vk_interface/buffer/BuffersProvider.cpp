#include "BuffersProvider.hpp"

#include <vk_interface/buffer/ProvidedBuffer.hpp>

#include <utility>
#include <algorithm>

#include <vk_interface\Device.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\resources\ResourcesController.hpp>

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
    BufferResourceHandle bufferRes = resourcesController_->CreateBuffer(bufferDesc);
    auto* providedBuffer = new ProvidedBuffer{ bufferRes, buffersCount };
    providedBuffers_.push_back(providedBuffer);


    VkDevice const device = device_->Handle();

    VkBufferView view = VK_NULL_HANDLE;
    VkBufferViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;

    std::uint32_t prevOffset = 0;
    for (auto i = 0u; i < buffersCount; ++i) {
        viewInfo.flags = VK_FLAGS_NONE;
        viewInfo.buffer = bufferRes.GetResource()->handle_;
        viewInfo.format = format;
        //viewInfo.offset = desc[i].offset_;
        viewInfo.offset = prevOffset;
        prevOffset += desc[i].size_;
        viewInfo.range = desc[i].size_;

        if (format != VK_FORMAT_UNDEFINED) {
            VK_ASSERT(table_->vkCreateBufferView(device, &viewInfo, nullptr, &view));
        }
        
        BufferView* resultView = new BufferView{ view, format, viewInfo.offset, viewInfo.range, providedBuffer };
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

        if (view.handle_ != VK_NULL_HANDLE) {
            table_->vkDestroyBufferView(device, view.handle_, nullptr);
        }
        
        delete *viewIt;
        bufferViews_.erase(viewIt);


        if (--providedBuffer.referenceCount_ == 0) {
            resourcesController_->FreeBuffer(providedBuffer.bufferResource_);
            delete *providedBufferIt;
            providedBuffers_.erase(providedBufferIt);
        }

    }

}

BufferView* BuffersProvider::GetView(BufferViewHandle handle)
{
    return handle.view_;
}

BufferResource* BuffersProvider::GetViewResource(BufferViewHandle handle)
{
    return handle.view_->providedBuffer_->bufferResource_.GetResource();
}

BuffersProvider::~BuffersProvider()
{
    VkDevice const device = device_->Handle();
    for (auto const& view : bufferViews_) {
        table_->vkDestroyBufferView(device, view->handle_, nullptr);
        delete view;
    }

    for (auto const& providedBuffer : providedBuffers_) {
        resourcesController_->FreeBuffer(providedBuffer->bufferResource_);
        delete providedBuffer;
    }
}

}