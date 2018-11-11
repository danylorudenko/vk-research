#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "DescriptorSet.hpp"
#include "..\image\ImageView.hpp"
#include "..\buffer\BufferView.hpp"
#include "..\resources\ResourcesController.hpp"

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