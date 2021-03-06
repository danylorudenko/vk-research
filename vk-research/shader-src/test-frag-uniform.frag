#version 450 core

layout(location = 0) in vec3 out_v_norm;

layout(location = 0) out vec4 finalColor;

const vec3 light_vec = vec3(1.0, 0.0, 0.0);

const float lambertian = 1.0 / 3.14;

void main()
{
	vec3 normN = normalize(out_v_norm);
	float ndotl = dot(normN, light_vec);
	finalColor = vec4(vec3(ndotl), 1.0);
}