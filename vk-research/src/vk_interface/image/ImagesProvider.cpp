#include "ImagesProvider.hpp"
#include "../Device.hpp"
#include "../ImportTable.hpp"
#include "../resources/ResourcesController.hpp"

#include <utility>

namespace VKW
{

ImagesProvider::ImagesProvider()
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{

}

ImagesProvider::ImagesProvider(ImagesProviderDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , resourcesController_{ desc.resourcesController_ }
{

}

ImagesProvider::ImagesProvider(ImagesProvider&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , resourcesController_{ nullptr }
{
    operator=(std::move(rhs));
}

ImagesProvider& ImagesProvider::operator=(ImagesProvider&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(resourcesController_, rhs.resourcesController_);
    std::swap(imageViews_, rhs.imageViews_);

    return *this;
}

}