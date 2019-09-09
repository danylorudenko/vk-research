#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

#include "..\renderer\Material.hpp"
#include "..\transform\TansformComponent.hpp"

#include "..\renderer\CustomTempBlurPass.hpp"

#include <utility>
#include <cstdio>

#include <glm\gtc\quaternion.hpp>

#include <imgui/imgui.h>
#include "ImGuiUserData.hpp"

VulkanApplicationDelegate::VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, std::uint32_t buffering, bool vkDebug, bool imguiEnabled)
    : mainWindow_ {
        instance,
        title,
        windowWidth,
        windowHeight,
        "VulkanRenderWindow",
        VulkanApplicationDelegate::WinProc,
        this }
    , inputSystem_{ mainWindow_.NativeHandle() }
    , imguiEnabled_{ imguiEnabled }
{
    VKW::LoaderDesc loaderDesc;
    loaderDesc.hInstance_ = instance; 
    loaderDesc.hwnd_ = mainWindow_.NativeHandle();
    loaderDesc.bufferingCount_ = buffering;
    loaderDesc.ioManager_ = &ioManager_;
    loaderDesc.debug_ = vkDebug;

    vulkanLoader_ = std::make_unique<VKW::Loader>(loaderDesc);



    Render::RootDesc rootDesc;
    rootDesc.loader_ = vulkanLoader_.get();
    rootDesc.resourceProxy_ = vulkanLoader_->resourceRendererProxy_.get();
    rootDesc.renderPassController_ = vulkanLoader_->renderPassController_.get();
    rootDesc.imagesProvider_ = vulkanLoader_->imagesProvider_.get();
    rootDesc.framedDescriptorsHub_ = vulkanLoader_->framedDescriptorsHub_.get();
    rootDesc.layoutController_ = vulkanLoader_->descriptorLayoutController_.get();
    rootDesc.shaderModuleFactory_ = vulkanLoader_->shaderModuleFactory_.get();
    rootDesc.pipelineFactory_ = vulkanLoader_->pipelineFactory_.get();
    rootDesc.presentationController_ = vulkanLoader_->presentationController_.get();
    rootDesc.mainWorkerTemp_ = vulkanLoader_->workersProvider_->GetWorker(VKW::WorkerType::GRAPHICS_PRESENT, 0);

    renderRoot_ = std::make_unique<Render::Root>(rootDesc);

    ImGuiHelperDesc imguiHelperDesc;
    imguiHelperDesc.window_ = &mainWindow_;
    imguiHelperDesc.inputSystem_ = &inputSystem_;
    imguiHelperDesc.root_ = renderRoot_.get();
    imguiHelper_ = std::make_unique<ImGuiHelper>(imguiHelperDesc);
}

VulkanApplicationDelegate::~VulkanApplicationDelegate()
{

}

LRESULT VulkanApplicationDelegate::WinProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    auto* appDelegate = reinterpret_cast<VulkanApplicationDelegate*>(::GetWindowLongPtr(handle, GWLP_USERDATA));
    
    switch (message)
    {
    case WM_INPUT:
    {
        UINT code = GET_RAWINPUT_CODE_WPARAM(wparam);
        LRESULT result;
        if (code == RIM_INPUTSINK || code == RIM_INPUT)
            result = ::DefWindowProc(handle, message, wparam, lparam);

        appDelegate->GetInputSystem().ProcessSystemInput(handle, wparam, lparam);
        return result;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    
    return ::DefWindowProc(handle, message, wparam, lparam);
}

InputSystem& VulkanApplicationDelegate::GetInputSystem()
{
    return inputSystem_;
}

void VulkanApplicationDelegate::start()
{
    FakeParseRendererResources();

    if (imguiEnabled_)
        InitImGui();
}



void VulkanApplicationDelegate::update()
{
    ////////////////////////////////////////////////////
    // Frame time measurement
    auto currTime = std::chrono::high_resolution_clock::now();
    auto frameTime = currTime - prevFrameTimePoint_;
    prevFrameTimePoint_ = currTime;
    prevFrameDelta_ = std::chrono::duration_cast<std::chrono::milliseconds>(frameTime).count();


    inputSystem_.Update();

    VKW::PresentationContext presentationContext = renderRoot_->AcquireNextPresentationContext();
    std::uint32_t context = presentationContext.contextId_;
    ////////////////////////////////////////////////////
    //
    if (imguiEnabled_)
        imguiHelper_->BeginFrame(context);



    //
    ////////////////////////////////////////////////////
    Transform::TransformSystemCameraData cameraData;
    cameraData.cameraPos = glm::vec3(0.0f);
    cameraData.cameraEuler = glm::vec3(0.0f);
    cameraData.cameraFowDegrees = 60.0f;

    VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
    VKW::ImageResource* colorBufferResource = renderRoot_->ResourceProxy()->GetResource(colorBufferView->resource_);
    cameraData.width = (float)colorBufferResource->width_;
    cameraData.height = (float)colorBufferResource->height_;

    transformationSystem_.Update(context, cameraData);

    VKW::WorkerFrameCommandReciever commandReciever = renderRoot_->BeginRenderGraph(presentationContext);
    renderRoot_->IterateRenderGraph(presentationContext, commandReciever);

    ImGuiUser(context);


    if (imguiEnabled_) {
        imguiHelper_->EndFrame(context);
        imguiHelper_->Render(context, commandReciever);
    }

    renderRoot_->EndRenderGraph(presentationContext, commandReciever);
}

void VulkanApplicationDelegate::shutdown()
{
    
}

void VulkanApplicationDelegate::FakeParseRendererResources()
{

}

void VulkanApplicationDelegate::InitImGui()
{
    if (imguiEnabled_)
        imguiHelper_->Init();
}



void VulkanApplicationDelegate::ImGuiUser(std::uint32_t context)
{
    if (imguiEnabled_) {
        IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!"); // Exceptionally add an extra assert here for people confused with initial dear imgui setup
        
        VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
        VKW::ImageResource* colorBufferResource = renderRoot_->ResourceProxy()->GetResource(colorBufferView->resource_);

        //ImGui::SetNextWindowContentWidth(100.0f);
        ImGui::SetNextWindowPos(ImVec2((float)colorBufferResource->width_ - 10.0f, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGuiWindowFlags frameDataWindowFlags = 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoInputs;
        static bool frameDataOpened = false;
        if (ImGui::Begin("Frame Stats", nullptr, frameDataWindowFlags))
        {
            ImGui::Text("DT: %f", prevFrameDelta_ * 0.001f);
            ImGui::Text("FPS: %f", 1.0f / prevFrameDelta_);
            ImGui::End();
        }
        

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
        
        ImGuiWindowFlags blurWindowFlags = 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Blur scale", nullptr, blurWindowFlags))
        {
            ImGui::SetNextItemWidth(100.0f);
            ImGui::SliderFloat("", &IMGUI_USER_BLUR_SCALE, 0.0f, 1.0f);
            
            ImGui::RadioButton("Fast blur", (int*)&IMGUI_USER_BLUR, (int)IMGUI_USER_BLUR_TYPE_FAST); ImGui::SameLine();
            ImGui::RadioButton("Full blur", (int*)&IMGUI_USER_BLUR, (int)IMGUI_USER_BLUR_TYPE_FULL); ImGui::SameLine();

            ImGui::End();
        }


        //static bool p_open = true;
        //ImGui::ShowDemoWindow(&p_open);
    }
    
}
