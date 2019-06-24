#version 450 core

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 finalColor;

layout(set = 0, binding = 0) uniform sampler2D backgroundTexture;

void main()
{
	finalColor = texture(backgroundTexture, in_uv);
}
