#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 4) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;


void main()
{
	out_uv = in_uv;
	gl_Position = vec4(in_pos.x, in_pos.y, 0.99, 1.0);
}