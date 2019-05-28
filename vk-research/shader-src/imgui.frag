#version 450 core

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 finalColor;

layout(set = 0, binding = 0) uniform sampler2D textureAtlas;

void main()
{
	finalColor = in_color * texture(textureAtlas, in_uv).x;
}
