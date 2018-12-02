#include "PresentationController.hpp"

#include "..\Tools.hpp"
#include "..\ImportTable.hpp"
#include "..\Device.hpp"
#include "..\Swapchain.hpp"
#include "..\worker\Worker.hpp"
#include <utility>
#include <limits>

namespace VKW
{

PresentationController::PresentationController()
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
    , presentCompleteSemaphore_{ VK_NULL_HANDLE }
{

}

PresentationController::PresentationController(PresentationControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , swapchain_{ desc.swapchain_ }
    , presentationWorker_{ desc.presentationWorker_ }
    , presentCompleteSemaphore_{ VK_NULL_HANDLE }
{
    VkSemaphoreCreateInfo sInfo;
    sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sInfo.pNext = nullptr;
    sInfo.flags = VK_FLAGS_NONE;

    VK_ASSERT(table_->vkCreateSemaphore(device_->Handle(), &sInfo, nullptr, &presentCompleteSemaphore_));
}

PresentationController::PresentationController(PresentationController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
    , presentCompleteSemaphore_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

PresentationController& PresentationController::operator=(PresentationController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(swapchain_, rhs.swapchain_);
    std::swap(presentationWorker_, rhs.presentationWorker_);

    std::swap(presentCompleteSemaphore_, rhs.presentCompleteSemaphore_);

    return *this;
}

PresentationController::~PresentationController()
{
    if (presentCompleteSemaphore_) {
        table_->vkDestroySemaphore(device_->Handle(), presentCompleteSemaphore_, nullptr);
        presentCompleteSemaphore_ = VK_NULL_HANDLE;
    }
}

std::uint32_t PresentationController::AcquireNewContextId()
{
    std::uint32_t imageIndex = 0;

    VK_ASSERT(table_->vkAcquireNextImageKHR(
        device_->Handle(), 
        swapchain_->Handle(), 
        std::numeric_limits<std::uint64_t>::max(),
        presentCompleteSemaphore_,
        VK_NULL_HANDLE,
        &imageIndex)
    );

    return imageIndex;

}

void PresentationController::PresentContextId(std::uint32_t contextId, WorkerFrameCompleteSemaphore frameRenderingCompleteSemaphore)
{
    VkQueue presentQueue = presentationWorker_->QueueHandle();
    VkSwapchainKHR swapchain = swapchain_->Handle();
    std::uint32_t imageIndex = contextId;

    VkSemaphore waitSemaphore = frameRenderingCompleteSemaphore.frameCompleteSemaphore_;

    VkPresentInfoKHR pInfo;
    pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pInfo.pNext = nullptr;
    pInfo.waitSemaphoreCount = 1;
    pInfo.pWaitSemaphores = &waitSemaphore;
    pInfo.swapchainCount = 1;
    pInfo.pSwapchains = &swapchain;
    pInfo.pImageIndices = &imageIndex;

    VK_ASSERT(table_->vkQueuePresentKHR(presentQueue, &pInfo));

}

}