#version 460 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D opaquebeauty;

void main()
{
	out_color.rgb = texture(opaquebeauty, uv).rgb;
	out_color.a = 1;
}