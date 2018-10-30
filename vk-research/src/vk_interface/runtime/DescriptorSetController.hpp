#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "DescriptorSet.hpp"
#include "../image/ImageView.hpp"
#include "../buffer/BufferView.hpp"
#include "../resources/ResourcesController.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;
class ImagesProvider;
class BuffersProvider;
class DescriptorLayoutController;


struct DescriptorDesc
{
    DescriptorType type_;

    union {
        struct {
            ImageViewHandle imageViewHandle_;
            VkSampler sampler_;
            ImageUsage usage_;
        } imageDesc;

        struct {
            BufferViewHandle bufferViewHandle_;
        } bufferView;

        struct {
            BufferResourceHandle bufferResource_;
            std::uint32_t offset_;
            std::uint32_t size_;
        } bufferInfo;
    };
};

struct DescriptorSetDesc
{
    DescriptorSetLayoutHandle layout_;
    DescriptorDesc members_[DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};



struct DescriptorSetControllerDesc
{
    ImportTable* table_;
    Device* device_;
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;
};

class DescriptorSetController
    : public NonCopyable
{
    DescriptorSetController();
    DescriptorSetController(DescriptorSetControllerDesc const& desc);

    DescriptorSetController(DescriptorSetController&& rhs);
    DescriptorSetController& operator=(DescriptorSetController&& rhs);

    ~DescriptorSetController();

private:
    struct DescriptorWriteInfo
    {
        VkDescriptorImageInfo imageInfo;
        VkDescriptorBufferInfo bufferInfo;
        VkBufferView bufferView;
    };

    void AssembleSetCreateInfo(VkDescriptorSet dstSet, DescriptorSetDesc const& desc, VkWriteDescriptorSet* results);

    static void DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkImageView view);
    static void DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkSampler sampler);
    static void DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkBufferView view);
    static void DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteInfo& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size);

private:
    ImportTable* table_;
    Device* device_;

    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;


    std::vector<DescriptorSet*> descriptorSets_;
};

}