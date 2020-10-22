#include "PresentationController.hpp"

#include <vk_interface\Tools.hpp>
#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Swapchain.hpp>
#include <vk_interface\worker\Worker.hpp>

#include <utility>
#include <limits>

namespace VKW
{

PresentationController::PresentationController()
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
    , nextSemaphore_{ 0 }
{

}

PresentationController::PresentationController(PresentationControllerDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
    , swapchain_{ desc.swapchain_ }
    , presentationWorker_{ desc.presentationWorker_ }
    , nextSemaphore_{ 0 }
{
    VkSemaphoreCreateInfo sInfo;
    sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sInfo.pNext = nullptr;
    sInfo.flags = VK_FLAGS_NONE;

    VkDevice const device = device_->Handle();

    std::uint32_t const swapchainImagesCount = swapchain_->ImageCount();
    std::uint32_t const semaphoresCount = swapchainImagesCount * 2;
    for (auto i = 0u; i < semaphoresCount; ++i) {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        ERR_GUARD_VK(table_->vkCreateSemaphore(device, &sInfo, nullptr, &semaphore));
        semaphoresQueue_.emplace_back(semaphore);
    }

    
}

PresentationController::PresentationController(PresentationController&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
    , swapchain_{ nullptr }
    , presentationWorker_{ nullptr }
    , nextSemaphore_{ 0 }
{
    operator=(std::move(rhs));
}

PresentationController& PresentationController::operator=(PresentationController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);
    std::swap(swapchain_, rhs.swapchain_);
    std::swap(presentationWorker_, rhs.presentationWorker_);

    std::swap(nextSemaphore_, rhs.nextSemaphore_);
    std::swap(semaphoresQueue_, rhs.semaphoresQueue_);

    return *this;
}

PresentationController::~PresentationController()
{
    VkDevice const device = device_->Handle();
    for (auto& semaphore : semaphoresQueue_) {
        table_->vkDestroySemaphore(device, semaphore, nullptr);
    }
}

PresentationContext PresentationController::AcquireNewPresentationContext()
{
    std::uint32_t const currentSemaphoreId = (nextSemaphore_++ % static_cast<std::uint32_t>(semaphoresQueue_.size()));
    VkSemaphore currentSemaphore = semaphoresQueue_[currentSemaphoreId];

    std::uint32_t imageIndex = 0;

    ERR_GUARD_VK(table_->vkAcquireNextImageKHR(
        device_->Handle(), 
        swapchain_->Handle(), 
        std::numeric_limits<std::uint64_t>::max() / 2, // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkAcquireNextImageKHR: timeout must not be UINT64_MAX
        currentSemaphore,
        VK_NULL_HANDLE,
        &imageIndex)
    );

    return PresentationContext{ imageIndex, currentSemaphore };
}

void PresentationController::PresentContextId(std::uint32_t contextId, WorkerFrameCompleteSemaphore frameRenderingCompleteSemaphore)
{
    VkQueue presentQueue = presentationWorker_->QueueHandle();
    VkSwapchainKHR swapchain = swapchain_->Handle();
    std::uint32_t imageIndex = contextId;

    VkSemaphore waitSemaphore = frameRenderingCompleteSemaphore.frameCompleteSemaphore_;

    VkResult results = VK_SUCCESS;

    VkPresentInfoKHR pInfo;
    pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pInfo.pNext = nullptr;
    pInfo.waitSemaphoreCount = 1;
    pInfo.pWaitSemaphores = &waitSemaphore;
    pInfo.swapchainCount = 1;
    pInfo.pSwapchains = &swapchain;
    pInfo.pImageIndices = &imageIndex;
    pInfo.pResults = &results;

    ERR_GUARD_VK(table_->vkQueuePresentKHR(presentQueue, &pInfo));

}

}