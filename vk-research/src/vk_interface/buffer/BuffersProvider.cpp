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

void BuffersProvider::AcquireBuffers(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferViewHandle* results)
{
    auto const prevSize = bufferViews_.size();
    bufferViews_.resize(prevSize + buffersCount);
    
    VkBufferViewCreateInfo bvInfo;
    bvInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvInfo.pNext = nullptr;
    for (auto i = 0u; i < buffersCount; ++i) {
        BufferResource* resource = resourcesController_->GetBuffer(desc[i].buffer_);

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
        bv.resource_ = desc[i].buffer_;
        
        results[i].id_ = static_cast<std::uint32_t>(prevSize + i);
    }
}

void BuffersProvider::ReleaseBuffers(std::uint32_t buffersCount, BufferViewHandle const* handles)
{

}

}