#include "ImGuiHelper.hpp"
#include "..\renderer\Root.hpp"

#include <utility>
#include <cstdint>

char const* ImGuiHelper::IMGUI_TEXTURE_KEY = "imtex";
char const* ImGuiHelper::IMGUI_TEXTURE_STAGING_BUFFER_KEY = "imupb";

char const* ImGuiHelper::IMGUI_SET_LAYOUT_KEY = "imlyt";
char const* ImGuiHelper::IMGUI_PIPELINE_KEY = "imppl";
char const* ImGuiHelper::IMGUI_PASS_KEY = "impss";
char const* ImGuiHelper::IMGUI_MATERIAL_TEMPLATE_KEY = "immt";
char const* ImGuiHelper::IMGUI_MATERIAL_KEY = "imml";

char const* ImGuiHelper::IMGUI_VERT_SHADER_KEY = "imvs";
char const* ImGuiHelper::IMGUI_FRAG_SHADER_KEY = "imfs";

char const* ImGuiHelper::IMGUI_VERTEX_BUFFER_KEY = "imvt";
char const* ImGuiHelper::IMGUI_INDEX_BUFFER_KEY = "imin";

ImGuiHelper::ImGuiHelper()
    : root_{ nullptr }
{
}

ImGuiHelper::ImGuiHelper(ImGuiHelperDesc const& desc)
    : root_{ desc.root_ }
{
}

ImGuiHelper::ImGuiHelper(ImGuiHelper&& rhs)
    : root_{ nullptr }
{
    operator=(std::move(rhs));
}

ImGuiHelper& ImGuiHelper::operator=(ImGuiHelper&& rhs)
{
    std::swap(root_, rhs.root_);
    std::swap(mainRenderWorkItem_, rhs.mainRenderWorkItem_);

    return *this;
}

ImGuiHelper::~ImGuiHelper()
{
    // hmmmmmmmm
}

void ImGuiHelper::Init(std::uint32_t viewportWidth, std::uint32_t viewportHeight)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_None;
    io.DisplaySize.x = static_cast<float>(viewportWidth);
    io.DisplaySize.y = static_cast<float>(viewportHeight);

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

    io.Fonts->SetTexID((ImTextureID)IMGUI_TEXTURE_KEY);



    Render::RenderPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0].resourceKey_ = Render::Root::SWAPCHAIN_IMAGE_KEY;
    passDesc.colorAttachments_[0].usage_ = VKW::RENDER_PASS_ATTACHMENT_USAGE_COLOR_PRESERVE_PRESENT;

    Render::ShaderDesc vertexShaderDesc;
    vertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexShaderDesc.relativePath_ = "shader-src\\imgui.vert.spv";

    Render::ShaderDesc fragmentShaderDesc;
    fragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentShaderDesc.relativePath_ = "shader-src\\imgui.frag.spv";

    Render::GraphicsPipelineDesc pipelineDesc;

    pipelineDesc.renderPass_ = IMGUI_PASS_KEY;
    pipelineDesc.dynamicStateFlags_ = 
    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0] = IMGUI_VERT_SHADER_KEY;
    pipelineDesc.shaderStages_[1] = IMGUI_FRAG_SHADER_KEY;

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
    setLayoutDesc.membersDesc_[0].type_ = VKW::DescriptorType::DESCRIPTOR_TYPE_TEXTURE;
    setLayoutDesc.membersDesc_[0].binding_ = 0;

    Render::PipelineLayoutDesc layoutDesc;
    layoutDesc.staticMembersCount_ = 0;
    layoutDesc.instancedMembersCount_ = 1;
    layoutDesc.instancedMembers_[0] = IMGUI_SET_LAYOUT_KEY;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineDesc.vertexInputInfo_ = &vInfo;
    pipelineDesc.viewportInfo_ = &vpInfo;
    pipelineDesc.depthStencilInfo_ = &dsInfo;
    pipelineDesc.layoutDesc_ = &layoutDesc;
    //pipelineDesc.dynamicStateFlags_ = VKW::PIPELINE_DYNAMIC_STATE_VIEWPORT | VKW::PIPELINE_DYNAMIC_STATE_SCISSOR;
    pipelineDesc.dynamicStateFlags_ = VK_FLAGS_NONE;

    VKW::BufferViewDesc vertexBufferDesc;
    vertexBufferDesc.usage_ = VKW::BufferUsage::VERTEX_INDEX_WRITABLE;
    vertexBufferDesc.size_ = IMGUI_VERTEX_BUFFER_SIZE;
    vertexBufferDesc.format_ = VK_FORMAT_UNDEFINED;

    VKW::BufferViewDesc indexBufferDesc;
    indexBufferDesc.usage_ = VKW::BufferUsage::VERTEX_INDEX_WRITABLE;
    indexBufferDesc.size_ = IMGUI_INDEX_BUFFER_SIZE;
    indexBufferDesc.format_ = VK_FORMAT_UNDEFINED;

    Render::RenderWorkItemDesc renderWorkItemDesc;
    renderWorkItemDesc.vertexCount_ = 0;
    renderWorkItemDesc.vertexBufferKey_ = IMGUI_VERTEX_BUFFER_KEY;
    renderWorkItemDesc.indexCount_ = 0;
    renderWorkItemDesc.indexBufferKey_ = IMGUI_INDEX_BUFFER_KEY;
    renderWorkItemDesc.indexBindOffset_ = 0;
    renderWorkItemDesc.setOwnerDescs_->members_[0].texture2D_.imageKey_ = IMGUI_TEXTURE_KEY;

    Render::MaterialTemplateDesc materialTemplateDesc;
    materialTemplateDesc.perPassDataCount_ = 1;
    materialTemplateDesc.perPassData_[0].passKey_ = IMGUI_PASS_KEY;
    materialTemplateDesc.perPassData_[0].pipelineKey_ = IMGUI_PIPELINE_KEY;

    Render::MaterialDesc materialDesc;
    materialDesc.templateKey_ = IMGUI_MATERIAL_TEMPLATE_KEY;


    root_->DefineRenderPass(IMGUI_PASS_KEY, passDesc);
    root_->DefineShader(IMGUI_VERT_SHADER_KEY, vertexShaderDesc);
    root_->DefineShader(IMGUI_FRAG_SHADER_KEY, fragmentShaderDesc);
    root_->DefineSetLayout(IMGUI_SET_LAYOUT_KEY, setLayoutDesc);
    root_->DefineGraphicsPipeline(IMGUI_PIPELINE_KEY, pipelineDesc);
    root_->DefineMaterialTemplate(IMGUI_MATERIAL_TEMPLATE_KEY, materialTemplateDesc);
    root_->DefineMaterial(IMGUI_MATERIAL_KEY, materialDesc);
    root_->DefineGlobalBuffer(IMGUI_INDEX_BUFFER_KEY, indexBufferDesc);
    root_->DefineGlobalBuffer(IMGUI_VERTEX_BUFFER_KEY, vertexBufferDesc);

    root_->RegisterMaterial(IMGUI_MATERIAL_KEY);
    //root_->PushPassTemp(IMGUI_PASS_KEY); NONONONONO, we gonna process our pass ourselves

    mainRenderWorkItem_ =  root_->ConstructRenderWorkItem(IMGUI_PIPELINE_KEY, renderWorkItemDesc);
}

void ImGuiHelper::BeginFrame(std::uint32_t context)
{
    ImGui::NewFrame();
}

void ImGuiHelper::EndFrame(std::uint32_t context)
{
    ImGui::EndFrame();
}

void ImGuiHelper::Render(std::uint32_t context, VKW::WorkerFrameCommandReciever commandReciever)
{
    ImGui::Render();
    ImDrawData* data = ImGui::GetDrawData();

    std::int32_t const totalVertexSizeBytes = data->TotalVtxCount * sizeof(ImDrawVert);
    std::int32_t const totalIndexSizeBytes = data->TotalIdxCount * sizeof(ImDrawIdx);

    assert(totalVertexSizeBytes <= IMGUI_VERTEX_BUFFER_SIZE && "ImGui vertex buffer overflows allocated space.");
    assert(totalIndexSizeBytes <= IMGUI_INDEX_BUFFER_SIZE && "ImGui index buffer overflows allocated space.");

    void* mappedVertexBuffer = root_->MapBuffer(IMGUI_VERTEX_BUFFER_KEY, context);
    void* mappedIndexBuffer = root_->MapBuffer(IMGUI_INDEX_BUFFER_KEY, context);
    std::uint32_t indexBindOffset = 0;
    Render::RenderWorkItem* renderWorkItem = root_->FindRenderWorkItem(IMGUI_PIPELINE_KEY, mainRenderWorkItem_);

    Render::Pass& pass = root_->FindPass(IMGUI_PASS_KEY);
    pass.Begin(context, &commandReciever);

    std::int32_t const cmdListsCount = data->CmdListsCount;
    for (std::int32_t i = 0; i < cmdListsCount; ++i) {
        ImDrawList const* drawList = data->CmdLists[0];
        ImVector<ImDrawVert> const& vertexBuffer = drawList->VtxBuffer;
        ImVector<ImDrawIdx> const& indexBuffer = drawList->IdxBuffer;
        ImVector<ImDrawCmd> const& cmdBuffer = drawList->CmdBuffer;

        std::uint32_t const vertexBytesCount = vertexBuffer.size() * sizeof(ImDrawVert);
        std::memcpy(mappedVertexBuffer, vertexBuffer.Data, vertexBytesCount);
        mappedVertexBuffer = reinterpret_cast<std::uint8_t*>(mappedVertexBuffer) + vertexBytesCount;
        std::uint32_t verticesCountBytes = vertexBuffer.size();

        std::uint32_t const indexBytesCount = indexBuffer.size() * sizeof(ImDrawIdx);
        std::memcpy(mappedIndexBuffer, indexBuffer.Data, indexBytesCount);
        mappedIndexBuffer = reinterpret_cast<std::uint8_t*>(mappedIndexBuffer) + indexBytesCount;
        indexBindOffset += indexBuffer.size();

        renderWorkItem->indexBindOffset_ = indexBindOffset;
        renderWorkItem->vertexCount_ = vertexBuffer.size();

        std::int32_t const commandsCount = cmdBuffer.size();
        for (std::int32_t cmdI = 0; cmdI < commandsCount; ++cmdI) {
            ImDrawCmd const& drawCmd = cmdBuffer[cmdI];
            if (drawCmd.UserCallback != nullptr) {
                drawCmd.UserCallback(drawList, &drawCmd);
            }
            else {
                // drawCmd.TextureId // can be safely ingored, since we don't use multiple fonts or images
                renderWorkItem->indexCount_ = drawCmd.ElemCount;
                //drawCmd.
                pass.Render(context, &commandReciever); // this needs attention
            }
        }
    }

    pass.End(context, &commandReciever);

    root_->FlushBuffer(IMGUI_VERTEX_BUFFER_KEY, context);
    root_->FlushBuffer(IMGUI_INDEX_BUFFER_KEY, context);
}

