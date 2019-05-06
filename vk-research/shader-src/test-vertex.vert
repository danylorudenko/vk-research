#version 450 core

layout(location = 0) in vec3 v_pos;

layout (set=0, binding=0) uniform TransformUniform
{
	mat4 mvp_mat;
} transformUniform;


void main()
{
	gl_Position = transformUniform.mvp_mat * vec4(v_pos, 1.0);
}