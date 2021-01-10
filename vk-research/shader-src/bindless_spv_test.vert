#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 in_pos;
layout(location = 1) in int  in_id;

layout(set = 0, binding = 0) uniform sampler2D textureArray[];

layout(set = 1, binding = 0) uniform TextureIdBlock
{
	int id;
} textureId;

void main()
{
	float dynamic_value = texture(textureArray[textureId.id], in_pos.xy).r;
	gl_Position = vec4(in_pos.x, dynamic_value, 0.0, 1.0);
}