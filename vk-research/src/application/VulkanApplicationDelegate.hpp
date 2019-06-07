#pragma once

#include <memory>

#include "Application.hpp"
#include "..\class_features\NonMovable.hpp"

#include "..\system\Window.hpp"
#include "..\system\DynamicLibrary.hpp"
#include "..\input\InputSystem.hpp"

#include "..\io\IOManager.hpp"
#include "..\vk_interface\Loader.hpp"
#include "..\renderer\Root.hpp"
#include "..\renderer\RootDef.hpp"
#include "..\renderer\UniformBufferWriterProxy.hpp"
#include "..\transform\TransformationSystem.hpp"
#include "ImGuiHelper.hpp"

struct CustomData
{
    Render::UniformBufferWriterProxy uniformProxy_;
    Transform::TransformComponent* transformComponent_;
};

class VulkanApplicationDelegate
    : public Application::ApplicationDelegate
    , public NonMovable
{
public:
    CustomData customData_;

public:
    VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, std::uint32_t buffering, bool vkDebug, bool imguiEnabled);
    
    virtual void start() override;
    virtual void update() override;
    virtual void shutdown() override;

    virtual ~VulkanApplicationDelegate();

    static LRESULT CALLBACK WinProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

private:
    void InitImGui();
    void TestImGui(std::uint32_t context);

private:
    Window mainWindow_;
    InputSystem inputSystem_;
    IOManager ioManager_;

    std::unique_ptr<VKW::Loader> vulkanLoader_;
    std::unique_ptr<Render::Root> renderRoot_;
    std::unique_ptr<ImGuiHelper> imguiHelper_;
    bool imguiEnabled_;


    Transform::TransformationSystem transformationSystem_;
    

    // TODO
    void FakeParseRendererResources();
};