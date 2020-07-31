#version 450 core
out vec4 out_color;

uniform float pathsalpha;

void main()
{
	out_color = vec4(1.0f, 1.0f, 1.0f, pathsalpha);
}