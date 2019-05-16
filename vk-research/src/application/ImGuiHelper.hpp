#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\renderer\RenderWorkItem.hpp"

namespace Render
{
class Root;
}

struct ImGuiHelperDesc
{
    Render::Root* root_;
};

class ImGuiHelper
{
public:
    ImGuiHelper();
    ImGuiHelper(ImGuiHelperDesc const& desc);
    ImGuiHelper(ImGuiHelper&& rhs);

    ImGuiHelper& operator=(ImGuiHelper&& rhs);

    ~ImGuiHelper();

    void Init(std::uint32_t viewportWidth, std::uint32_t viewportHeight);
    void BeginFrame(std::uint32_t context);
    void EndFrame(std::uint32_t context);
    void Render(std::uint32_t context);


private:
    static char const* IMGUI_TEXTURE_KEY;
    static char const* IMGUI_TEXTURE_STAGING_BUFFER_KEY;

    static char const* IMGUI_SET_LAYOUT_KEY;
    static char const* IMGUI_PIPELINE_KEY;
    static char const* IMGUI_PASS_KEY;

    static char const* IMGUI_VERT_SHADER_KEY;
    static char const* IMGUI_FRAG_SHADER_KEY;

    static char const* IMGUI_VERTEX_BUFFER_KEY;
    static char const* IMGUI_INDEX_BUFFER_KEY;

    static constexpr std::uint32_t IMGUI_VERTEX_BUFFER_SIZE = 20 * 4096;
    static constexpr std::uint32_t IMGUI_INDEX_BUFFER_SIZE = 4 * 2048;


    Render::Root* root_;
    Render::RenderWorkItemHandle mainRenderWorkItem_;
    
};