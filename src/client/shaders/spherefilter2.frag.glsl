#version 450 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D scenebeautytex;
uniform sampler2D masktex;

void main() 
{
	vec4 beauty = texture(scenebeautytex, uv);
	float mask = .33f*texture(masktex, uv).r;

	out_color = (1-mask)*beauty + mask*vec4(0,0,1,1);
}