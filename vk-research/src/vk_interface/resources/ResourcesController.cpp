#include <utility>
#include "ResourcesController.hpp"
#include "../memory/MemoryController.hpp"
#include "../Device.hpp"
#include "../Tools.hpp"

namespace VKW
{

ResourcesController::ResourcesController()
    : table_{ nullptr }
    , device_{ nullptr }
    , memoryController_{ nullptr }
{
}

ResourcesController::ResourcesController(ResourcesControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , memoryController_{ desc.memoryController_ }
{

}

ResourcesController::ResourcesController(ResourcesController&& rhs)
{
    operator=(std::move(rhs));
}

ResourcesController& ResourcesController::operator=(ResourcesController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(memoryController_, rhs.memoryController_);

    std::swap(buffers_, rhs.buffers_);
    std::swap(images_, rhs.images_);
    
    return *this;
}

ResourcesController::~ResourcesController()
{
    for (auto& staticBuffer : buffers_) {
        auto memory = staticBuffer.memory_;
        table_->vkDestroyBuffer(device_->Handle(), staticBuffer.handle_, nullptr);
        memoryController_->ReleaseMemoryRegion(staticBuffer.memory_);
    }
}

BufferResourceHandle ResourcesController::CreateBuffer(BufferDesc const& desc)
{
    VkBufferCreateInfo vkBufferCreateInfo;
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = nullptr;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCreateInfo.size = desc.size_;
    vkBufferCreateInfo.flags = VK_FLAGS_NONE;
    vkBufferCreateInfo.usage = VK_FLAGS_NONE; // temp


    MemoryPageRegionDesc regionDesc;

    switch (desc.usage_)
    {
    case BufferUsage::VERTEX_INDEX:
        regionDesc.usage_ = MemoryUsage::VERTEX_INDEX;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        break;
    case BufferUsage::UNIFORM:
        regionDesc.usage_ = MemoryUsage::UNIFORM;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        break;
    case BufferUsage::UPLOAD_BUFFER:
        regionDesc.usage_ = MemoryUsage::UPLOAD_BUFFER;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        break;
    }


    VkBuffer vkBuffer = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateBuffer(device_->Handle(), &vkBufferCreateInfo, nullptr, &vkBuffer));

    VkMemoryRequirements memoryRequirements;
    table_->vkGetBufferMemoryRequirements(device_->Handle(), vkBuffer, &memoryRequirements);

    regionDesc.size_ = memoryRequirements.size;
    regionDesc.alignment_ = memoryRequirements.alignment;
    regionDesc.memoryTypeBits_ = memoryRequirements.memoryTypeBits;
    assert(IsPowerOf2(regionDesc.alignment_) && "Alignemnt is not power of 2!");


    MemoryRegion memoryRegion;
    memoryController_->ProvideMemoryRegion(regionDesc, memoryRegion);
    assert((memoryRegion.pageHandle_.id_ != MemoryRegion{}.pageHandle_.id_) && "Couldn't find memoryRegion");

    MemoryPage const& page = memoryController_->GetPage(memoryRegion.pageHandle_);
    assert(memoryRequirements.memoryTypeBits & (1 << page.memoryTypeId_) && "MemoryRegion has invalid memoryType");
    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), vkBuffer, page.deviceMemory_, memoryRegion.offset_));
    buffers_.emplace_back(vkBuffer, static_cast<std::uint32_t>(desc.size_), memoryRegion);

    return { static_cast<std::uint32_t>(buffers_.size()) - 1 };
}

ImageResourceHandle ResourcesController::CreateImage(ImageDesc const& desc)
{
    VkImageCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.format = desc.format_;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.width = desc.width_;
    info.extent.height = desc.height_;
    info.extent.depth = 1;
    info.arrayLayers = 1;
    info.mipLevels = 1;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    MemoryPageRegionDesc memoryDesc;

    switch (desc.usage_)
    {
    case ImageUsage::TEXTURE:
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        memoryDesc.usage_ = MemoryUsage::SAMPLE_TEXTURE;
        break;
        
    case ImageUsage::RENDER_TARGET:
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        memoryDesc.usage_ = MemoryUsage::COLOR_ATTACHMENT;
        break;

    case ImageUsage::DEPTH_STENCIL:
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        memoryDesc.usage_ = MemoryUsage::DEPTH_STENCIL_ATTACHMENT;
        break;

    default:
        assert(false && "Non-supported usage for image.");
        // TODO
        break;
    }

    VkImage image = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateImage(device_->Handle(), &info, nullptr, &image));

    VkMemoryRequirements memoryRequirements;
    table_->vkGetImageMemoryRequirements(device_->Handle(), image, &memoryRequirements);

    memoryDesc.size_ = memoryRequirements.size;
    memoryDesc.alignment_ = memoryRequirements.alignment;
    memoryDesc.memoryTypeBits_ = memoryRequirements.memoryTypeBits;

    MemoryRegion region;
    memoryController_->ProvideMemoryRegion(memoryDesc, region);
    assert(region.pageHandle_.id_ != MemoryPageHandle{}.id_ && "Couldn't find appropriate memoryRegion");

    MemoryPage const& page = memoryController_->GetPage(region.pageHandle_);
    assert((memoryRequirements.memoryTypeBits & (1 << page.memoryTypeId_)) && "MemoryRegion has invalid memoryType");
    VK_ASSERT(table_->vkBindImageMemory(device_->Handle(), image, page.deviceMemory_, region.offset_));
    images_.emplace_back(image, desc.format_, desc.width_, desc.height_, region);

    return { static_cast<std::uint32_t>(images_.size()) - 1 };
}

void ResourcesController::FreeBuffer(BufferResourceHandle handle)
{
    auto& buffer = buffers_[handle.id_];
    table_->vkDestroyBuffer(device_->Handle(), buffer.handle_, nullptr);
    buffers_.erase(buffers_.begin() + handle.id_);
}

void ResourcesController::FreeImage(ImageResourceHandle handle)
{
    auto& image = images_[handle.id_];
    table_->vkDestroyImage(device_->Handle(), image.handle_, nullptr);
    images_.erase(images_.begin() + handle.id_);
}

BufferResource* ResourcesController::GetBuffer(BufferResourceHandle handle)
{
    return buffers_.data() + handle.id_;
}

ImageResource* ResourcesController::GetImage(ImageResourceHandle handle)
{
    return images_.data() + handle.id_;
}

}