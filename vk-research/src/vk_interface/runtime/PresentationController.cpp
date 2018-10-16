#include "PresentationController.hpp"

#include <utility>

namespace VKW
{

PresentationController::PresentationController()
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
{

}

PresentationController::PresentationController(PresentationControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , swapchain_{ desc.swapchain_ }
    , presentationWorker_{ desc.presentationWorker_ }
{

}

PresentationController::PresentationController(PresentationController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
{
    operator=(std::move(rhs));
}

PresentationController& PresentationController::operator=(PresentationController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(swapchain_, rhs.swapchain_);
    std::swap(presentationWorker_, rhs.presentationWorker_);

    return *this;
}

}