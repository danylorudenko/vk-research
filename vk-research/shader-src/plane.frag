#version 450 core

layout(location = 0) in vec3 in_wpos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent_viewpos;
layout(location = 3) in vec3 in_tangent_wpos;
layout(location = 4) in vec2 in_uv;
layout(location = 5) in vec3 in_tangent_light;

layout(location = 0) out vec4 finalColor;

layout(set = 0, binding = 0) uniform sampler2D planeTexture;
layout(set = 0, binding = 1) uniform sampler2D bumpTexture;
layout(set = 0, binding = 2) uniform sampler2D normalTexture;

layout(set = 1, binding = 0, std140) uniform TransformUniform
{
	mat4 mvp_mat;
	mat4 model_mat;
	vec4 view_dir;
	vec4 view_pos;
} transformUniform;

void main()
{
	float bump = texture(bumpTexture, in_uv).r;
	
	vec3 view_dir_local = normalize(in_tangent_viewpos - in_tangent_wpos);
	vec2 offset = view_dir_local.xy / view_dir_local.z * (bump * 0.035);
	vec2 sampleLocation = in_uv + offset;
	
	vec3 diffuseTexture = texture(planeTexture, sampleLocation).rgb;
	vec3 textureNormal = texture(normalTexture, sampleLocation).rgb;
	
	vec3 normal = normalize(textureNormal * 2.0 - 1.0);
	
	float diffuseTerm = dot(normal, in_tangent_light) + 0.3;
	
	finalColor = vec4(diffuseTerm * diffuseTexture, 1.0);
	//finalColor = vec4(normal, 1.0);
}
