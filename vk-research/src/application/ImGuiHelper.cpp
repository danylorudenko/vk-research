#include <application\ImGuiHelper.hpp>

#include <input\InputSystem.hpp>
#include <system\Window.hpp>
#include <renderer\Root.hpp>

#include <glm\vec2.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <utility>
#include <cstdint>

#include <Windows.h>

char const* ImGuiHelper::IMGUI_TEXTURE_KEY = "imtex";
char const* ImGuiHelper::IMGUI_TEXTURE_STAGING_BUFFER_KEY = "imupb";

char const* ImGuiHelper::IMGUI_MATERIAL_SET_LAYOUT_KEY = "immlt";
char const* ImGuiHelper::IMGUI_ITEM_SET_LAYOUT_KEY = "imlyt";
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
    : window_{ desc.window_ }
    , inputSystem_{ desc.inputSystem_ }
    , root_{ desc.root_ }
{
}

ImGuiHelper::ImGuiHelper(ImGuiHelper&& rhs)
    : root_{ nullptr }
{
    operator=(std::move(rhs));
}

ImGuiHelper& ImGuiHelper::operator=(ImGuiHelper&& rhs)
{
    std::swap(window_, rhs.window_);
    std::swap(inputSystem_, rhs.inputSystem_);
    std::swap(root_, rhs.root_);
    std::swap(mainRenderWorkItem_, rhs.mainRenderWorkItem_);

    return *this;
}

ImGuiHelper::~ImGuiHelper()
{
    // hmmmmmmmm
}

void ImGuiHelper::Init()
{
    VAL::ImageView* colorBufferView = root_->FindGlobalImage(root_->GetDefaultSceneColorOutput(), 0);
    VAL::ImageResource* colorBufferResource = root_->ResourceProxy()->GetResource(colorBufferView->resource_);

    std::uint32_t viewportWidth = colorBufferResource->width_ - root_->GetDefaultSceneColorBufferThreeshold();
    std::uint32_t viewportHeight = colorBufferResource->height_ - root_->GetDefaultSceneColorBufferThreeshold();
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(viewportWidth);
    io.DisplaySize.y = static_cast<float>(viewportHeight);

    io.ConfigFlags = ImGuiConfigFlags_None;
    //io.FontGlobalScale = 2.5f;
    io.IniFilename = nullptr;

    io.KeyMap[ImGuiKey_Tab] = (std::int32_t)Keys::Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = (std::int32_t)Keys::Left;
    io.KeyMap[ImGuiKey_RightArrow] = (std::int32_t)Keys::Right;
    io.KeyMap[ImGuiKey_UpArrow] = (std::int32_t)Keys::Up;
    io.KeyMap[ImGuiKey_DownArrow] = (std::int32_t)Keys::Down;
    io.KeyMap[ImGuiKey_PageUp] = (std::int32_t)Keys::PageUp;
    io.KeyMap[ImGuiKey_PageDown] = (std::int32_t)Keys::PageDown;
    io.KeyMap[ImGuiKey_Home] = (std::int32_t)Keys::Home;
    io.KeyMap[ImGuiKey_End] = (std::int32_t)Keys::End;
    io.KeyMap[ImGuiKey_Insert] = (std::int32_t)Keys::Insert;
    io.KeyMap[ImGuiKey_Delete] = (std::int32_t)Keys::Delete;
    io.KeyMap[ImGuiKey_Backspace] = (std::int32_t)Keys::Backspace;
    io.KeyMap[ImGuiKey_Space] = (std::int32_t)Keys::Space;
    io.KeyMap[ImGuiKey_Enter] = (std::int32_t)Keys::Enter;
    io.KeyMap[ImGuiKey_Escape] = (std::int32_t)Keys::Escape;
    io.KeyMap[ImGuiKey_A] = (std::int32_t)Keys::A;         // for text edit CTRL+A: select all
    io.KeyMap[ImGuiKey_C] = (std::int32_t)Keys::C;         // for text edit CTRL+C: copy
    io.KeyMap[ImGuiKey_V] = (std::int32_t)Keys::V;         // for text edit CTRL+V: paste
    io.KeyMap[ImGuiKey_X] = (std::int32_t)Keys::X;         // for text edit CTRL+X: cut
    io.KeyMap[ImGuiKey_Y] = (std::int32_t)Keys::Y;         // for text edit CTRL+Y: redo
    io.KeyMap[ImGuiKey_Z] = (std::int32_t)Keys::Z;


    // getting ready imgui atlas
    int imguiAtlasWidth = 0, imguiAtlasHeight, imguiPixelBytes = 0;
    unsigned char* textureData = nullptr;
    io.Fonts->GetTexDataAsAlpha8(&textureData, &imguiAtlasWidth, &imguiAtlasHeight, &imguiPixelBytes);
    assert(imguiPixelBytes == 1 && "ImGuiHelper: bytesPerPixel > 1, something went wrong");

    VAL::ImageViewDesc imageDesc;
    imageDesc.width_ = imguiAtlasWidth;
    imageDesc.height_ = imguiAtlasHeight;
    imageDesc.format_ = VK_FORMAT_R8_UNORM;
    imageDesc.usage_ = VAL::ImageUsage::TEXTURE;
    root_->DefineGlobalImage(IMGUI_TEXTURE_KEY, imageDesc);

    VAL::BufferViewDesc stagingBufferDesc;
    stagingBufferDesc.format_ = VK_FORMAT_UNDEFINED;
    stagingBufferDesc.size_ = imguiAtlasWidth * imguiAtlasHeight * imguiPixelBytes;
    stagingBufferDesc.usage_ = VAL::BufferUsage::UPLOAD_BUFFER;
    root_->DefineGlobalBuffer(IMGUI_TEXTURE_STAGING_BUFFER_KEY, stagingBufferDesc);

    void* stagingBufferPtr = root_->MapBuffer(IMGUI_TEXTURE_STAGING_BUFFER_KEY, 0);
    std::memcpy(stagingBufferPtr, textureData, imguiAtlasWidth * imguiAtlasHeight * imguiPixelBytes);
    root_->FlushBuffer(IMGUI_TEXTURE_STAGING_BUFFER_KEY, 0);

    root_->CopyStagingBufferToGPUTexture(IMGUI_TEXTURE_STAGING_BUFFER_KEY, IMGUI_TEXTURE_KEY, 0);

    VAL::ImageView* imguiTextureView = root_->FindGlobalImage(IMGUI_TEXTURE_KEY, 0);
    VAL::ImageResource* imguiTextureResource = root_->ResourceProxy()->GetResource(imguiTextureView->resource_);
    VkImageLayout targetLayout = VK_IMAGE_LAYOUT_GENERAL;
    root_->ImageLayoutTransition(0, 1, &imguiTextureResource->handle_, &targetLayout);

    io.Fonts->SetTexID((ImTextureID)IMGUI_TEXTURE_KEY);



    Render::RootGraphicsPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0].resourceKey_ = root_->GetDefaultSceneColorOutput();
    passDesc.colorAttachments_[0].usage_ = VAL::RENDER_PASS_ATTACHMENT_USAGE_COLOR_PRESERVE;

    Render::ShaderDesc vertexShaderDesc;
    vertexShaderDesc.type_ = VAL::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexShaderDesc.relativePath_ = "shader-src\\imgui.vert.spv";

    Render::ShaderDesc fragmentShaderDesc;
    fragmentShaderDesc.type_ = VAL::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentShaderDesc.relativePath_ = "shader-src\\imgui.frag.spv";

    Render::GraphicsPipelineDesc pipelineDesc;

    pipelineDesc.renderPass_ = IMGUI_PASS_KEY;
    pipelineDesc.dynamicStateFlags_ = 
    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0] = IMGUI_VERT_SHADER_KEY;
    pipelineDesc.shaderStages_[1] = IMGUI_FRAG_SHADER_KEY;

    VAL::InputAssemblyInfo iaInfo;
    iaInfo.primitiveRestartEnable_ = false;
    iaInfo.primitiveTopology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VAL::VertexInputInfo vInfo;
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

    VAL::ViewportInfo vpInfo;
    vpInfo.viewportsCount_ = 1;
    VAL::ViewportInfo::Viewport& vp = vpInfo.viewports_[0];
    vp.x_ = static_cast<float>(root_->GetDefaultSceneColorBufferThreeshold());
    vp.y_ = static_cast<float>(root_->GetDefaultSceneColorBufferThreeshold());
    vp.width_ = static_cast<float>(viewportWidth);
    vp.height_ = static_cast<float>(viewportHeight);
    vp.minDepth_ = 0.0f;
    vp.maxDepth_ = 1.0f;
    vp.scissorXoffset_ = 0;
    vp.scissorYoffset_ = 0;
    vp.scissorXextent_ = viewportWidth;
    vp.scissorYextent_ = viewportHeight;

    VAL::DepthStencilInfo dsInfo;
    dsInfo.depthTestEnabled_ = true;
    dsInfo.depthWriteEnabled_ = true;
    dsInfo.depthCompareOp_ = VK_COMPARE_OP_LESS;
    dsInfo.stencilTestEnabled_ = false;
    dsInfo.backStencilState_ = {};
    dsInfo.frontStencilState_ = {};


    VAL::DescriptorSetLayoutDesc materialSetLayoutDesc;
    materialSetLayoutDesc.stage_ = VAL::DescriptorStage::RENDERING;
    materialSetLayoutDesc.membersCount_ = 1;
    materialSetLayoutDesc.membersDesc_[0].type_ = VAL::DescriptorType::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialSetLayoutDesc.membersDesc_[0].binding_ = 0;

    VAL::DescriptorSetLayoutDesc itemSetLayoutDesc;
    itemSetLayoutDesc.stage_ = VAL::DescriptorStage::RENDERING;
    itemSetLayoutDesc.membersCount_ = 1;
    itemSetLayoutDesc.membersDesc_[0].type_ = VAL::DescriptorType::DESCRIPTOR_TYPE_TEXTURE;
    itemSetLayoutDesc.membersDesc_[0].binding_ = 0;

    Render::PipelineLayoutDesc layoutDesc;
    layoutDesc.staticMembersCount_ = 1;
    layoutDesc.staticMembers_[0] = IMGUI_MATERIAL_SET_LAYOUT_KEY;
    layoutDesc.instancedMembersCount_ = 1;
    layoutDesc.instancedMembers_[0] = IMGUI_ITEM_SET_LAYOUT_KEY;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineDesc.vertexInputInfo_ = &vInfo;
    pipelineDesc.viewportInfo_ = &vpInfo;
    pipelineDesc.depthStencilInfo_ = &dsInfo;
    pipelineDesc.layoutDesc_ = &layoutDesc;
    //pipelineDesc.dynamicStateFlags_ = VAL::PIPELINE_DYNAMIC_STATE_VIEWPORT | VAL::PIPELINE_DYNAMIC_STATE_SCISSOR;
    pipelineDesc.dynamicStateFlags_ = VK_FLAGS_NONE;
    pipelineDesc.blendingState_ = VAL::PIPELINE_BLENDING_SRC_ALPHA_DST_ONE;

    VAL::BufferViewDesc vertexBufferDesc;
    vertexBufferDesc.usage_ = VAL::BufferUsage::VERTEX_INDEX_WRITABLE;
    vertexBufferDesc.size_ = IMGUI_VERTEX_BUFFER_SIZE;
    vertexBufferDesc.format_ = VK_FORMAT_UNDEFINED;

    VAL::BufferViewDesc indexBufferDesc;
    indexBufferDesc.usage_ = VAL::BufferUsage::VERTEX_INDEX_WRITABLE;
    indexBufferDesc.size_ = IMGUI_INDEX_BUFFER_SIZE;
    indexBufferDesc.format_ = VK_FORMAT_UNDEFINED;

    Render::RenderWorkItemDesc renderWorkItemDesc;
    renderWorkItemDesc.vertexCount_ = 0;
    renderWorkItemDesc.vertexBufferKey_ = IMGUI_VERTEX_BUFFER_KEY;
    renderWorkItemDesc.vertexBindOffset_ = 0;
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
    materialDesc.perPassData_[0].setOwnerDesc_->members_[0].uniformBuffer_.size_ = 256;


    root_->DefineRenderPass(IMGUI_PASS_KEY, passDesc);
    root_->DefineShader(IMGUI_VERT_SHADER_KEY, vertexShaderDesc);
    root_->DefineShader(IMGUI_FRAG_SHADER_KEY, fragmentShaderDesc);
    root_->DefineSetLayout(IMGUI_ITEM_SET_LAYOUT_KEY, itemSetLayoutDesc);
    root_->DefineSetLayout(IMGUI_MATERIAL_SET_LAYOUT_KEY, materialSetLayoutDesc);
    root_->DefineGraphicsPipeline(IMGUI_PIPELINE_KEY, pipelineDesc);
    root_->DefineMaterialTemplate(IMGUI_MATERIAL_TEMPLATE_KEY, materialTemplateDesc);
    root_->DefineMaterial(IMGUI_MATERIAL_KEY, materialDesc);
    root_->DefineGlobalBuffer(IMGUI_VERTEX_BUFFER_KEY, vertexBufferDesc);
    root_->DefineGlobalBuffer(IMGUI_INDEX_BUFFER_KEY, indexBufferDesc);

    root_->RegisterMaterial(IMGUI_MATERIAL_KEY);
    //root_->PushPass(IMGUI_PASS_KEY); NONONONONO, we gonna process our pass ourselves

    Render::Material& material = root_->FindMaterial(IMGUI_MATERIAL_KEY);
    transformUniformBufferProxy_ = Render::UniformBufferWriterProxy{ root_, IMGUI_MATERIAL_KEY, 0, 0, 0 };


    mainRenderWorkItem_ = root_->ConstructRenderWorkItem(IMGUI_PIPELINE_KEY, renderWorkItemDesc);
}

void ImGuiHelper::BeginFrame(std::uint32_t context)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    HWND activeWindow = ::GetForegroundWindow();
    if (activeWindow == window_->NativeHandle()) {
        POINT pos;
        if (::GetCursorPos(&pos) && ::ScreenToClient(activeWindow, &pos)) {
            io.MousePos = ImVec2((float)pos.x, (float)pos.y);
        }
    }

    io.MouseDown[0] = inputSystem_->GetLeftMouseButtonPressed();
    io.MouseDown[1] = inputSystem_->GetRightMouseButtonPressed();
    io.MouseDown[2] = inputSystem_->GetMiddleMouseButtonPressed();

    io.KeyCtrl = inputSystem_->GetKeyboardButtonDown(Keys::Ctrl);
    io.KeyShift = inputSystem_->GetKeyboardButtonDown(Keys::Shift);
    io.KeyAlt = inputSystem_->GetKeyboardButtonDown(Keys::Alt);
    io.KeySuper = inputSystem_->GetKeyboardButtonDown(Keys::WinLeft) || inputSystem_->GetKeyboardButtonDown(Keys::WinRight);

    std::int32_t keyBegin = (std::int32_t)Keys::BEGIN;
    std::int32_t keyEnd = (std::int32_t)Keys::END;
    for (std::int32_t key = keyBegin; key < keyEnd; ++key) {
        io.KeysDown[key] = inputSystem_->GetKeyboardButtonDown((Keys)key);

        if (inputSystem_->GetKeyboardButtonJustReleased((Keys)key)) {
            char result = (char)InputSystem::GetCharFromKeys((Keys)key);
            inputSystem_->GetKeyboardButtonJustReleased((Keys)key);
            if(result)
                io.AddInputCharacter(result);
        }
    }

    ImGui::NewFrame();
}

void ImGuiHelper::EndFrame(std::uint32_t context)
{
    ImGui::EndFrame();
}

void ImGuiHelper::Render(std::uint32_t context, VAL::WorkerFrameCommandReciever commandReciever)
{
    ImGui::Render();
    ImDrawData* data = ImGui::GetDrawData();

    ImVec2 imDisplayPos = data->DisplayPos;
    ImVec2 imDisplaySize = data->DisplaySize;

    glm::vec2 displayPos{ imDisplayPos.x, imDisplayPos.y };
    glm::vec2 displaySize{ imDisplaySize.x, imDisplaySize.y };
    
    std::uint8_t* transformUniformPtr = transformUniformBufferProxy_.MappedPtr<std::uint8_t>(context);
    std::memcpy(transformUniformPtr, glm::value_ptr(displayPos), sizeof(displayPos));
    std::memcpy(transformUniformPtr + sizeof(displayPos), glm::value_ptr(displaySize), sizeof(displaySize));

    std::int32_t const totalVertexSizeBytes = data->TotalVtxCount * sizeof(ImDrawVert);
    std::int32_t const totalIndexSizeBytes = data->TotalIdxCount * sizeof(ImDrawIdx);

    assert(totalVertexSizeBytes <= IMGUI_VERTEX_BUFFER_SIZE && "ImGui vertex buffer overflows allocated space.");
    assert(totalIndexSizeBytes <= IMGUI_INDEX_BUFFER_SIZE && "ImGui index buffer overflows allocated space.");

    void* mappedVertexBuffer = root_->MapBuffer(IMGUI_VERTEX_BUFFER_KEY, context);
    void* mappedIndexBuffer = root_->MapBuffer(IMGUI_INDEX_BUFFER_KEY, context);
    std::uint32_t indexBindOffset = 0;
    std::uint32_t vertexBindOffset = 0;
    Render::RenderWorkItem* renderWorkItem = root_->FindRenderWorkItem(IMGUI_PIPELINE_KEY, mainRenderWorkItem_);

    Render::GraphicsPass& pass = reinterpret_cast<Render::GraphicsPass&>(root_->FindPass(IMGUI_PASS_KEY));
    pass.Begin(context, &commandReciever);

    std::int32_t const cmdListsCount = data->CmdListsCount;
    for (std::int32_t i = 0; i < cmdListsCount; ++i) {
        ImDrawList const* drawList = data->CmdLists[i];
        ImVector<ImDrawVert> const& vertexBuffer = drawList->VtxBuffer;
        ImVector<ImDrawIdx> const& indexBuffer = drawList->IdxBuffer;
        ImVector<ImDrawCmd> const& cmdBuffer = drawList->CmdBuffer;

        std::uint32_t const vertexBytesCount = vertexBuffer.size() * sizeof(ImDrawVert);
        std::memcpy(mappedVertexBuffer, vertexBuffer.Data, vertexBytesCount);
        mappedVertexBuffer = reinterpret_cast<std::uint8_t*>(mappedVertexBuffer) + vertexBytesCount;

        std::uint32_t const indexBytesCount = indexBuffer.size() * sizeof(ImDrawIdx);
        std::memcpy(mappedIndexBuffer, indexBuffer.Data, indexBytesCount);
        mappedIndexBuffer = reinterpret_cast<std::uint8_t*>(mappedIndexBuffer) + indexBytesCount;

        renderWorkItem->vertexCount_ = vertexBuffer.size();

        std::uint32_t indiciesRendered = 0;

        std::int32_t const commandsCount = cmdBuffer.size();
        for (std::int32_t cmdI = 0; cmdI < commandsCount; ++cmdI) {
            ImDrawCmd const& drawCmd = cmdBuffer[cmdI];
            if (drawCmd.UserCallback != nullptr) {
                drawCmd.UserCallback(drawList, &drawCmd);
            }
            else {
                // drawCmd.TextureId // can be safely ingored, since we don't use multiple fonts or images
                renderWorkItem->vertexBindOffset_ = vertexBindOffset;
                renderWorkItem->indexCount_ = drawCmd.ElemCount;
                renderWorkItem->indexBindOffset_ = indexBindOffset + (indiciesRendered * sizeof(ImDrawIdx));
                pass.Apply(context, &commandReciever);
                indiciesRendered += drawCmd.ElemCount;
            }
        }

        vertexBindOffset += vertexBuffer.size() * sizeof(ImDrawVert);
        indexBindOffset += indexBuffer.size() * sizeof(ImDrawIdx);
    }

    pass.End(context, &commandReciever);

    root_->FlushBuffer(IMGUI_VERTEX_BUFFER_KEY, context);
    root_->FlushBuffer(IMGUI_INDEX_BUFFER_KEY, context);
}

