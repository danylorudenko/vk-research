#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "../class_features/NonCopyable.hpp"

namespace VKW
{

class ImportTable;
class Instance;
class Device;

struct SwapchainDesc
{
    ImportTable* table_;
    Instance* instance_;
    Device* device_;

#ifdef _WIN32
    HINSTANCE hInstance_;
    HWND hwnd_;
#endif
};

class Swapchain
    : public NonCopyable
{
public:
    Swapchain();
    Swapchain(SwapchainDesc const& desc);

    Swapchain(Swapchain&& rhs);
    Swapchain& operator=(Swapchain&& rhs);

    ~Swapchain();

private:
    struct SwapChainImage
    {
        VkImage image_;
        VkImageView view_;
    };

    ImportTable* table_;
    Instance* instance_;
    Device* device_;

    VkSwapchainKHR swapchain_;
    std::vector<SwapChainImage> swapChainImages_;

};

}