#pragma once

#include <vector>
#include <vulkan\vulkan.h>
#include "..\class_features\NonCopyable.hpp"

namespace VKW
{

class ImportTable;
class Device;
class Surface;

struct SwapchainDesc
{
    ImportTable* table_;
    Device* device_;
    Surface* surface_;

    std::uint32_t imagesCount_;
};

class Swapchain
    : public NonCopyable
{
public:
    struct SwapchainImage
    {
        VkImage image_;
    };

public:
    Swapchain();
    Swapchain(SwapchainDesc const& desc);

    Swapchain(Swapchain&& rhs);
    Swapchain& operator=(Swapchain&& rhs);

    ~Swapchain();

    VkSwapchainKHR Handle() const;
    VkFormat Format() const;
    std::uint32_t Width() const;
    std::uint32_t Height() const;
    std::uint32_t ImageCount() const;
    SwapchainImage& Image(std::uint32_t index);


private:
    ImportTable* table_;
    Device* device_;
    Surface* surface_;

    VkSwapchainKHR swapchain_;
    VkSurfaceFormatKHR swapchainFormat_;
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t swapchainImageCount_;
    std::vector<SwapchainImage> swapchainImages_;

};

}