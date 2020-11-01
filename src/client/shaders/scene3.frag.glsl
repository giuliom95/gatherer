#version 460 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D finaltex;
uniform sampler2D transparentbeauty;

void main()
{
	vec3 dest = texture(finaltex, uv).rgb;
	vec4 src = texture(transparentbeauty, uv);
	out_color.rgb = src.a*src.rgb + (1-src.a)*dest.rgb;
	out_color.a = 1;
}