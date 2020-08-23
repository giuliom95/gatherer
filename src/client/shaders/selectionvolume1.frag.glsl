#version 450 core
out float out_mask;

uniform sampler2D scenedepth;

void main() 
{
	vec2 uv = gl_FragCoord.xy / 1024;
	float sd = texture(scenedepth, uv).r;
	if(gl_FragCoord.z > sd) discard;
	out_mask = 1;
}