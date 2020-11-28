#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec3 in_tan;
layout(location = 3) in vec3 in_btan;
layout(location = 4) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

layout(set = 1, binding = 0) uniform TransformUniform
{
	mat4 mvp_mat;
} transformUniform;


void main()
{
	out_uv = in_uv;
	//gl_Position = vec4(in_pos.x, in_pos.y, 0.99, 1.0);
	
	//out_v_norm = (transformUniform.mvp_mat * vec4(in_v_norm, 0.0)).xyz;
	gl_Position = transformUniform.mvp_mat * vec4(in_pos, 1.0);
}