#version 460 core
out float out_mask;

uniform sampler2D scenedepth;
uniform ivec2 framesize;

void main() 
{
	vec2 uv = gl_FragCoord.xy / vec2(framesize);
	float sd = texture(scenedepth, uv).r;
	if(gl_FragCoord.z > sd) discard;
	out_mask = 1;
}