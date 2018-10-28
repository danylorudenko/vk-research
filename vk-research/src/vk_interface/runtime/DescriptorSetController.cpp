#include "DescriptorSetController.hpp"
#include <utility>


namespace VKW
{

DescriptorSetController::DescriptorSetController()
    : table_{ nullptr }
    , device_{ nullptr }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{

}

DescriptorSetController::DescriptorSetController(DescriptorSetController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
	operator=(std::move(rhs));
}

DescriptorSetController& DescriptorSetController::operator=(DescriptorSetController&& rhs)
{
	std::swap(table_, rhs.table_);
	std::swap(device_, rhs.device_);
	std::swap(descriptorSets_, rhs.descriptorSets_);

	return *this;
}

DescriptorSetController::~DescriptorSetController()
{

}

}