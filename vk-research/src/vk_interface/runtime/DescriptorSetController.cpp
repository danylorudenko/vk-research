#include "DescriptorSetController.hpp"
#include <utility>
#include "../image/ImageView.hpp"


namespace VKW
{

DescriptorSetController::DescriptorSetController()
    : table_{ nullptr }
    , device_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , buffersProvider_{ desc.buffersProvider_ }
    , imagesProvider_{ desc.imagesProvider_ }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , buffersProvider_{ nullptr }
    , imagesProvider_{ nullptr }
{
	operator=(std::move(rhs));
}

DescriptorSetController& DescriptorSetController::operator=(DescriptorSetController&& rhs)
{
	std::swap(table_, rhs.table_);
	std::swap(device_, rhs.device_);
    std::swap(buffersProvider_, rhs.buffersProvider_);
    std::swap(imagesProvider_, rhs.imagesProvider_);
	std::swap(descriptorSets_, rhs.descriptorSets_);

	return *this;
}

DescriptorSetController::~DescriptorSetController()
{

}

}