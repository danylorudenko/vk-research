#include "DescriptorLayoutController.hpp"

#include <utility>

namespace VKW
{

DescriptorLayoutController::DescriptorLayoutController()
    : table_{ nullptr }
    , device_{ nullptr }
{

}

DescriptorLayoutController::DescriptorLayoutController(DescriptorLayoutControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{

}

DescriptorLayoutController::DescriptorLayoutController(DescriptorLayoutController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

DescriptorLayoutController& DescriptorLayoutController::operator=(DescriptorLayoutController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    return *this;
}

DescriptorLayoutController::~DescriptorLayoutController()
{

}

}