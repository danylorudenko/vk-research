#pragma once

#include "Application.hpp"
#include "..\class_features\NonMovable.hpp"

#include "..\system\Window.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "..\vk_interface\VulkanLoader.hpp"

class VulkanApplicationDelegate
    : public Application::ApplicationDelegate
    , public NonMovable
{
public:
    VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight);
    
    virtual void start() override;
    virtual void update() override;
    virtual void shutdown() override;

    virtual ~VulkanApplicationDelegate();

    static LRESULT CALLBACK WinProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

private:
    Window mainWindow_;
    VKW::VulkanLoader vulkanLoader_;
};