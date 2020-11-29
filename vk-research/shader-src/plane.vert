#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec3 in_tan;
layout(location = 3) in vec3 in_btan;
layout(location = 4) in vec2 in_uv;

layout(location = 0) out vec3 out_wpos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent_viewpos;
layout(location = 3) out vec3 out_tangent_wpos;
layout(location = 4) out vec2 out_uv;
layout(location = 5) out vec3 out_tangent_light;


layout(set = 1, binding = 0, std140) uniform TransformUniform
{
	mat4 mvp_mat;
	mat4 model_mat;
	vec4 view_dir;
	vec4 view_pos;
} transformUniform;


const vec3 C_LIGHT_DIR = vec3(1.0, 1.0, 1.0);

void main()
{	
	out_wpos = vec3(transformUniform.model_mat * vec4(in_pos, 1.0));
	
	vec3 T = mat3(transformUniform.model_mat) * in_tan;
	vec3 B = mat3(transformUniform.model_mat) * in_btan;
	vec3 N = mat3(transformUniform.model_mat) * in_norm;
	mat3 TBN = transpose(mat3(T, B, N));
	
	out_tangent_wpos = TBN * out_wpos;
	out_tangent_viewpos = TBN * vec3(transformUniform.view_pos);
	out_tangent_light = TBN * normalize(C_LIGHT_DIR);
	
	out_normal = in_norm;
	out_uv = in_uv;
	
	gl_Position = transformUniform.mvp_mat * vec4(in_pos, 1.0);
}