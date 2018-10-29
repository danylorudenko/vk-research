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


enum class DescriptorType
{
    SAMPLED_TEXTURE,
    UNIFORM_BUFFER
};

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


using DescriptorWriteProc = void(*)(DescriptorDesc const& src, VkWriteDescriptorSet& dst);


struct DescriptorSetControllerDesc
{
    ImportTable* table_;
    Device* device_;
    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;
};

class DescriptorSetController
    : public NonCopyable
{
    DescriptorSetController();
    DescriptorSetController(DescriptorSetControllerDesc const& desc);

    DescriptorSetController(DescriptorSetController&& rhs);
    DescriptorSetController& operator=(DescriptorSetController&& rhs);

    ~DescriptorSetController();

    static void DescriptorWriteProcSampledImage(DescriptorDesc const& src, VkWriteDescriptorSet& dst);
    static void DescriptorWriteProcUniformBuffer(DescriptorDesc const& src, VkWriteDescriptorSet& dst);

private:
    ImportTable* table_;
    Device* device_;

    BuffersProvider* buffersProvider_;
    ImagesProvider* imagesProvider_;

    std::vector<DescriptorSet*> descriptorSets_;
};

}