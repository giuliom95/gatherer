#version 450 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D colortex;

void main()
{
	out_color = texture(colortex, uv);
}