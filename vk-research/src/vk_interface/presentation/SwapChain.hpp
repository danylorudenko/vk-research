#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "../../class_features/NonCopyable.hpp"

namespace VKW
{

struct SwapChainDesc
{

};

class SwapChain
    : public NonCopyable
{
public:
    SwapChain();
    SwapChain(SwapChainDesc const& desc);

    SwapChain(SwapChain&& rhs);
    SwapChain& operator=(SwapChain&& rhs);

    ~SwapChain();

private:
    struct SwapChainImage
    {
        VkImage image_;
        VkImageView view_;
    };

    VkSwapchainKHR swapChain_;
    std::vector<SwapChainImage> swapChainImages_;

};

}