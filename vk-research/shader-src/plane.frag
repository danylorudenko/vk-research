#version 450 core

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 finalColor;

layout(set = 0, binding = 0) uniform sampler2D planeTexture;
layout(set = 0, binding = 1) uniform sampler2D bumpTexture;

void main()
{
	finalColor = vec4(texture(planeTexture, in_uv).rgb, 1.0);
}
