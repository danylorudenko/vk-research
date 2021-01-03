#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 in_pos;
layout(location = 1) in int  in_id;

layout(set = 0, binding = 0) uniform sampler2D textureArray[];

void main()
{
	float dynamic_value = texture(textureArray[in_id], in_pos.xy).r;
	gl_Position = vec4(in_pos, 1.0);
}