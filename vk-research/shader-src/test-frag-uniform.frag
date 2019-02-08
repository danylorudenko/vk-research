#version 450 core

layout(set=0, binding=0) uniform TestBuffer
{
	float testBufferElement;
} testBuffer;

layout(location = 0) out vec4 finalColor;

void main()
{
	finalColor = vec4(1.0, 0.5, 0.3, testBuffer.testBufferElement);
}