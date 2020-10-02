#include "ResourcesController.hpp"

#include <algorithm>

#include <vk_interface\memory\MemoryController.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Tools.hpp>

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
    VkDevice const device = device_->Handle();
    
    for (auto& bufferResource : buffers_) {
        table_->vkDestroyBuffer(device, bufferResource->handle_, nullptr);
        memoryController_->ReleaseMemoryRegion(bufferResource->memory_);
        delete bufferResource;
    }

    for (auto& imageResource : images_) {
        table_->vkDestroyImage(device, imageResource->handle_, nullptr);
        memoryController_->ReleaseMemoryRegion(imageResource->memory_);
        delete imageResource;
    }
}

BufferResourceHandle ResourcesController::CreateBuffer(BufferDesc const& desc)
{
    VkBufferCreateInfo vkBufferCreateInfo;
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = nullptr;
    vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCreateInfo.queueFamilyIndexCount = 0;
    vkBufferCreateInfo.pQueueFamilyIndices = nullptr;
    vkBufferCreateInfo.size = desc.size_;
    vkBufferCreateInfo.flags = VK_FLAGS_NONE;
    vkBufferCreateInfo.usage = VK_FLAGS_NONE; // temp value, assigned below


    MemoryPageRegionDesc regionDesc;

    switch (desc.usage_)
    {
    case BufferUsage::VERTEX_INDEX:
        regionDesc.memoryClass_ = MemoryClass::DeviceFast;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        break;
    case BufferUsage::VERTEX_INDEX_WRITABLE:
        regionDesc.memoryClass_ = MemoryClass::CpuStaging;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        break;
    case BufferUsage::UNIFORM:
        regionDesc.memoryClass_ = MemoryClass::CpuUniform;
        vkBufferCreateInfo.usage |= (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        break;
    case BufferUsage::UPLOAD_BUFFER:
        regionDesc.memoryClass_ = MemoryClass::CpuStaging;
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

    MemoryRegion memoryRegion = memoryController_->ProvideMemoryRegion(regionDesc);
    VK_ASSERT(table_->vkBindBufferMemory(device_->Handle(), vkBuffer, memoryRegion.pageHandle_.GetPage()->deviceMemory_, memoryRegion.offset_));

    BufferResource* resource = new BufferResource{ vkBuffer, static_cast<std::uint32_t>(desc.size_), memoryRegion };
    buffers_.emplace_back(resource);

    return BufferResourceHandle{ resource };
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
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices = nullptr;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.flags = VK_FLAGS_NONE;

    MemoryPageRegionDesc memoryDesc;

    switch (desc.usage_)
    {
    case ImageUsage::TEXTURE:
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        memoryDesc.memoryClass_ = MemoryClass::DeviceFast;
        break;
        
    case ImageUsage::STORAGE_IMAGE:
    case ImageUsage::STORAGE_IMAGE_READONLY:
        info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        memoryDesc.memoryClass_ = MemoryClass::DeviceFast;
        break;

    case ImageUsage::RENDER_TARGET:
        info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        memoryDesc.memoryClass_ = MemoryClass::DeviceFast;
        break;

    case ImageUsage::DEPTH:
    case ImageUsage::STENCIL:
    case ImageUsage::DEPTH_STENCIL:
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        memoryDesc.memoryClass_ = MemoryClass::DeviceFast;
        break;

    case ImageUsage::UPLOAD_IMAGE:
        info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        info.tiling = VK_IMAGE_TILING_LINEAR;
        memoryDesc.memoryClass_ = MemoryClass::CpuStaging;
        break;
    default:
        assert(false && "Non-supported usage for image.");
        break;
    }


    VkImage vkImage = VK_NULL_HANDLE;
    VK_ASSERT(table_->vkCreateImage(device_->Handle(), &info, nullptr, &vkImage));

    VkMemoryRequirements memoryRequirements;
    table_->vkGetImageMemoryRequirements(device_->Handle(), vkImage, &memoryRequirements);

    memoryDesc.size_ = memoryRequirements.size;
    memoryDesc.alignment_ = memoryRequirements.alignment;
    memoryDesc.memoryTypeBits_ = memoryRequirements.memoryTypeBits;

    MemoryRegion memoryRegion = memoryController_->ProvideMemoryRegion(memoryDesc);
    VK_ASSERT(table_->vkBindImageMemory(device_->Handle(), vkImage, memoryRegion.pageHandle_.GetPage()->deviceMemory_, memoryRegion.offset_));

    ImageResource* imageResource = new ImageResource{ vkImage, desc.format_, desc.width_, desc.height_, memoryRegion };
    images_.emplace_back(imageResource);

    return ImageResourceHandle{ imageResource };
}

void ResourcesController::FreeBuffer(BufferResourceHandle handle)
{
    auto bufferIt = std::find(buffers_.begin(), buffers_.end(), handle.GetResource());
    assert(bufferIt != buffers_.end() && "Can't free BufferResource.");

    auto& buffer = *bufferIt;

    table_->vkDestroyBuffer(device_->Handle(), buffer->handle_, nullptr);
    delete buffer;

    buffers_.erase(bufferIt);
}

void ResourcesController::FreeImage(ImageResourceHandle handle)
{
    // TODO this is for the case of swapchain. dirty hack
    if (handle.GetResource() == nullptr) {
        return;
    }
    
    auto imageIt = std::find(images_.cbegin(), images_.cend(), handle.GetResource());
    assert(imageIt != images_.end() && "Can't free ImageResource");

    auto& image = *imageIt;

    table_->vkDestroyImage(device_->Handle(), image->handle_, nullptr);
    delete image;

    images_.erase(imageIt);
}

}