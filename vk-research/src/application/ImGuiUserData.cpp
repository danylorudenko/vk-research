#include "ImGuiUserData.hpp"

float IMGUI_USER_BLUR_SCALE = 0.0f;
float IMGUI_USER_PLANE_POS[3] = { 0.0f, 0.0f, -1.0f };
float IMGUI_USER_PLANE_ROT[3] = { 0.0f, 0.0f, 0.0f };
float IMGUI_USER_CAMERA_POS[3] = { 0.0f, 0.0f, 0.0f };
float IMGUI_USER_CAMERA_ROT[3] = { 0.0f, 0.0f, 0.0f };

IMGUI_USER_BLUR_TYPE IMGUI_USER_BLUR = IMGUI_USER_BLUR_TYPE_FAST;