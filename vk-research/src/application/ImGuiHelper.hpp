#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\worker\WorkerFrame.hpp>
#include <imgui\imgui.h>

class Window;
class InputSystem;

struct ImGuiHelperDesc
{
    Window* window_;
    InputSystem* inputSystem_;
};

class ImGuiHelper
{
public:
    ImGuiHelper();
    ImGuiHelper(ImGuiHelperDesc const& desc);
    ImGuiHelper(ImGuiHelper&& rhs);

    ImGuiHelper& operator=(ImGuiHelper&& rhs);

    ~ImGuiHelper();

    void Init();
    void BeginFrame(std::uint32_t context);
    void EndFrame(std::uint32_t context);
    void Render(std::uint32_t context, VKW::WorkerFrameCommandReciever commandReciever);

private:
    static char const* IMGUI_TEXTURE_KEY;
    static char const* IMGUI_TEXTURE_STAGING_BUFFER_KEY;

    static char const* IMGUI_MATERIAL_SET_LAYOUT_KEY;
    static char const* IMGUI_ITEM_SET_LAYOUT_KEY;
    static char const* IMGUI_PIPELINE_KEY;
    static char const* IMGUI_PASS_KEY;
    static char const* IMGUI_MATERIAL_TEMPLATE_KEY;
    static char const* IMGUI_MATERIAL_KEY;

    static char const* IMGUI_VERT_SHADER_KEY;
    static char const* IMGUI_FRAG_SHADER_KEY;

    static char const* IMGUI_VERTEX_BUFFER_KEY;
    static char const* IMGUI_INDEX_BUFFER_KEY;

    static constexpr std::uint32_t IMGUI_VERTEX_BUFFER_SIZE = sizeof(ImDrawVert) * 2048 * 16;
    static constexpr std::uint32_t IMGUI_INDEX_BUFFER_SIZE = sizeof(ImDrawIdx) * 2048 * 16;

    Window* window_;
    InputSystem* inputSystem_;
    //Render::Root* root_;
    //Render::RenderWorkItemHandle mainRenderWorkItem_;
    //Render::UniformBufferWriterProxy transformUniformBufferProxy_;
    
};