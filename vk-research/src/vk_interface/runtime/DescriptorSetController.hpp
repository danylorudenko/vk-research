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
            BufferViewHandle pureBufferViewHandle_;
            std::uint32_t offset_;
            std::uint32_t size_;
        } bufferInfo;
    };
};

struct DescriptorSetDesc
{
    DescriptorSetLayoutHandle layout_;
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
public:
    DescriptorSetController();
    DescriptorSetController(DescriptorSetControllerDesc const& desc);

    DescriptorSetController(DescriptorSetController&& rhs);
    DescriptorSetController& operator=(DescriptorSetController&& rhs);

    DescriptorSetHandle AllocDescriptorSet(DescriptorSetDesc const& desc);
    void ReleaseDescriptorSet(DescriptorSetHandle handle);

    ~DescriptorSetController();

private:
    struct DescriptorWriteData
    {
        VkDescriptorImageInfo imageInfo;
        VkDescriptorBufferInfo bufferInfo;
        VkBufferView bufferView;
    };

    void AssembleSetCreateInfo(VkDescriptorSet dstSet, DescriptorSetDesc const& desc, VkWriteDescriptorSet* results);

    static void DecorateImageViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkImageView view);
    static void DecorateSamplerWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkSampler sampler);
    static void DecorateBufferViewWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBufferView view);
    static void DecorateBufferWriteDesc(VkWriteDescriptorSet& dst, DescriptorWriteData& dstInfo, VkBuffer buffer, std::uint32_t offset, std::uint32_t size);

private:
    ImportTable* table_;
    Device* device_;

    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
    DescriptorLayoutController* layoutController_;

    VkDescriptorPool pool_;

    std::vector<DescriptorSet*> descriptorSets_;
};

}