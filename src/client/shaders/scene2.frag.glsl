#version 450 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D beautytex;

void main()
{
	out_color.rgb = texture(beautytex, uv).rgb;
	out_color.a = 1;
}