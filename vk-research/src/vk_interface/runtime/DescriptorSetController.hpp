#pragma once

#include <class_features\NonCopyable.hpp>

#include <vk_interface\runtime\DescriptorSet.hpp>
#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\resources\ResourcesController.hpp>

#include <vector>

namespace VKW
{

class ImportTable;
class Device;
class ImagesProvider;
class BuffersProvider;
class DescriptorLayoutController;


struct DescriptorSetDesc
{
    DescriptorSetLayout* layout_;
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

    DescriptorSet* GetDescriptorSet(DescriptorSetHandle handle);

    ~DescriptorSetController();


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