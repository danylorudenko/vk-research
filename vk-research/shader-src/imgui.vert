#version 450 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

layout(set = 0, binding = 0) uniform TransformBuffer
{
	vec2 pos;
	vec2 screenSize;
} transform;

void main()
{
	out_uv = in_uv;
	out_color = in_color;
	gl_Position = vec4(in_pos.x / transform.screenSize.x, in_pos.y / transform.screenSize.y, 0.0, 1.0);
}