#version 460 core
in vec2 uv;

out vec3 out_color;

uniform sampler2D finaltex;

void main()
{
	out_color.rgb = texture(finaltex, uv).rgb;
}