#version 450 core

layout(location = 0) in vec3 in_v_pos;
layout(location = 1) in vec3 in_v_norm;

layout(location = 0) out vec3 out_v_norm;

layout (set=0, binding=0) uniform TransformUniform
{
	mat4 mvp_mat;
} transformUniform;


void main()
{
	out_v_norm = (transformUniform.mvp_mat * vec4(in_v_norm, 0.0)).xyz;
	gl_Position = transformUniform.mvp_mat * vec4(in_v_pos, 1.0);
}