#pragma once

#include <vector>
#include "../../class_features/NonCopyable.hpp"

#include "ImageView.hpp"


namespace VKW
{

class ImportTable;
class Device;
class ResourcesController;


struct ImageViewDesc
{

};


struct ImagesProviderDesc
{
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;
};

class ImagesProvider
{
public:
    ImagesProvider();
    ImagesProvider(ImagesProviderDesc const& desc);

    ImagesProvider(ImagesProvider&& rhs);
    ImagesProvider& operator=(ImagesProvider&& rhs);

    ~ImagesProvider();

private:
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;

    std::vector<ImageView> imageViews_;
};

}