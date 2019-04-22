#version 450 core

//layout(location = 0) in vec3 v_pos;

layout (set=0, binding=0) uniform OffsetUniform
{
	vec3 offset;
} offsetUniform;


const vec3 verts[3] = vec3[3](
	vec3(-0.5, 0.5, 0.0),
	vec3(0.5, 0.5, 0.0),
	vec3(0.0, -0.5, 0.0)
);

void main()
{
	gl_Position = vec4(verts[gl_VertexIndex] + offsetUniform.offset, 1.0f);
}