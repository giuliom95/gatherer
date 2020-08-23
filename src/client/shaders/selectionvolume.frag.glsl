#version 450 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D scenedepth;

void main() 
{
	out_color = vec4(.2f,.2f,1,.4f);
}