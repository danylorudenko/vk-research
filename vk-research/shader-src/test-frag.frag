#version 450 core

layout(location = 0) out vec4 finalColor;

in vec3 out_v_norm;

void main()
{
	finalColor = vec4(1.0, 0.5, 0.3, 1.0);
}