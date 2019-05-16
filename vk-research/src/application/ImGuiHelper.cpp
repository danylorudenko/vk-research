#include "ImGuiHelper.hpp"
#include "..\renderer\Root.hpp"

#include <utility>
#include <imgui/imgui.h>

char const* ImGuiHelper::IMGUI_TEXTURE_KEY = "imtex";
char const* ImGuiHelper::IMGUI_TEXTURE_STAGING_BUFFER_KEY = "imupb";

char const* ImGuiHelper::IMGUI_SET_LAYOUT_KEY = "imlyt";
char const* ImGuiHelper::IMGUI_PIPELINE_KEY = "imppl";
char const* ImGuiHelper::IMGUI_PASS_KEY = "impss";

char const* ImGuiHelper::IMGUI_VERT_SHADER_KEY = "imvs";
char const* ImGuiHelper::IMGUI_FRAG_SHADER_KEY = "imfs";

ImGuiHelper::ImGuiHelper()
    : root_{ nullptr }
    , mainWorkItem_{}
{
}

ImGuiHelper::ImGuiHelper(ImGuiHelperDesc const& desc)
    : root_{ desc.root_ }
    , mainWorkItem_{}
{
}

ImGuiHelper::ImGuiHelper(ImGuiHelper&& rhs)
    : root_{ nullptr }
    , mainWorkItem_{}
{
    operator=(std::move(rhs));
}

ImGuiHelper& ImGuiHelper::operator=(ImGuiHelper&& rhs)
{
    std::swap(root_, rhs.root_);
    std::swap(mainWorkItem_, rhs.mainWorkItem_);

    return *this;
}

void ImGuiHelper::Init(std::uint32_t viewportWidth, std::uint32_t viewportHeight)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_None;

    int imguiAtlasWidth = 0, imguiAtlasHeight = 0;
    unsigned char* textureData = nullptr;
    io.Fonts->GetTexDataAsRGBA32(&textureData, &imguiAtlasWidth, &imguiAtlasHeight);

    VKW::ImageViewDesc imageDesc;
    imageDesc.width_ = imguiAtlasWidth;
    imageDesc.height_ = imguiAtlasHeight;
    imageDesc.format_ = VK_FORMAT_R8G8B8A8_UNORM;
    imageDesc.usage_ = VKW::ImageUsage::TEXTURE;
    root_->DefineGlobalImage(IMGUI_TEXTURE_KEY, imageDesc);

    VKW::BufferViewDesc stagingBufferDesc;
    stagingBufferDesc.format_ = VK_FORMAT_UNDEFINED;
    stagingBufferDesc.size_ = imguiAtlasWidth * imguiAtlasHeight * 32;
    stagingBufferDesc.usage_ = VKW::BufferUsage::UPLOAD_BUFFER;
    root_->DefineGlobalBuffer(IMGUI_TEXTURE_STAGING_BUFFER_KEY, stagingBufferDesc);

    root_->CopyStagingBufferToGPUTexture(IMGUI_TEXTURE_STAGING_BUFFER_KEY, IMGUI_TEXTURE_KEY, 0);



    Render::RenderPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0] = Render::Root::SWAPCHAIN_IMAGE_KEY;

    root_->DefineRenderPass(IMGUI_PASS_KEY, passDesc);

    Render::Pass& pass = root_->FindPass(IMGUI_PASS_KEY);

    Render::ShaderDesc vertexShaderDesc;
    vertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexShaderDesc.relativePath_ = "shader-src\\imgui.vert.spv";

    Render::ShaderDesc fragmentShaderDesc;
    fragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentShaderDesc.relativePath_ = "shader-src\\imgui.frag.spv";

    root_->DefineShader(IMGUI_VERT_SHADER_KEY, vertexShaderDesc);
    root_->DefineShader(IMGUI_FRAG_SHADER_KEY, fragmentShaderDesc);

    Render::GraphicsPipelineDesc pipelineDesc;

    pipelineDesc.renderPass_ = IMGUI_PASS_KEY;
    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0] = "vShader";
    pipelineDesc.shaderStages_[1] = "fShader";

    VKW::InputAssemblyInfo iaInfo;
    iaInfo.primitiveRestartEnable_ = false;
    iaInfo.primitiveTopology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VKW::VertexInputInfo vInfo;
    vInfo.binding_ = 0;
    vInfo.stride_ = sizeof(ImDrawVert);
    vInfo.vertexAttributesCount_ = 3;
    vInfo.vertexAttributes_[0].location_ = 0;
    vInfo.vertexAttributes_[0].offset_ = offsetof(ImDrawVert, pos);
    vInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32_SFLOAT;
    vInfo.vertexAttributes_[1].location_ = 1;
    vInfo.vertexAttributes_[1].offset_ = offsetof(ImDrawVert, uv);
    vInfo.vertexAttributes_[1].format_ = VK_FORMAT_R32G32_SFLOAT;
    vInfo.vertexAttributes_[2].location_ = 2;
    vInfo.vertexAttributes_[2].offset_ = offsetof(ImDrawVert, col);
    vInfo.vertexAttributes_[2].format_ = VK_FORMAT_R8G8B8A8_UNORM;

    VKW::ViewportInfo vpInfo;
    vpInfo.viewportsCount_ = 1;
    VKW::ViewportInfo::Viewport& vp = vpInfo.viewports_[0];
    vp.x_ = 0.0f;
    vp.y_ = 0.0f;
    vp.width_ = static_cast<float>(viewportWidth);
    vp.height_ = static_cast<float>(viewportHeight);
    vp.minDepth_ = 0.0f;
    vp.maxDepth_ = 1.0f;
    vp.scissorXoffset_ = 0;
    vp.scissorYoffset_ = 0;
    vp.scissorXextent_ = viewportWidth;
    vp.scissorYextent_ = viewportHeight;

    VKW::DepthStencilInfo dsInfo;
    dsInfo.depthTestEnabled_ = true;
    dsInfo.depthWriteEnabled_ = true;
    dsInfo.depthCompareOp_ = VK_COMPARE_OP_LESS;
    dsInfo.stencilTestEnabled_ = false;
    dsInfo.backStencilState_ = {};
    dsInfo.frontStencilState_ = {};


    VKW::DescriptorSetLayoutDesc setLayoutDesc;
    setLayoutDesc.membersCount_ = 1;
    setLayoutDesc.membersDesc_[0].type_ = VKW::DescriptorType::DESCRIPTOR_TYPE_SAMPLED_TEXTURE;
    setLayoutDesc.membersDesc_[0].binding_ = 0;

    root_->DefineSetLayout(IMGUI_SET_LAYOUT_KEY, setLayoutDesc);

    Render::PipelineLayoutDesc layoutDesc;
    layoutDesc.staticMembersCount_ = 0;
    layoutDesc.staticMembers_[0] = IMGUI_SET_LAYOUT_KEY;
    layoutDesc.instancedMembersCount_ = 0;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineDesc.vertexInputInfo_ = &vInfo;
    pipelineDesc.viewportInfo_ = &vpInfo;
    pipelineDesc.depthStencilInfo_ = &dsInfo;
    pipelineDesc.layoutDesc_ = &layoutDesc;

    root_->DefineGraphicsPipeline(IMGUI_PIPELINE_KEY, pipelineDesc);
}

void ImGuiHelper::BeginFrame(std::uint32_t context)
{

}
