#version 460 core
out vec4 out_color;

uniform sampler2D scenedepth;
uniform sampler2D scenebeauty;
uniform ivec2 framesize;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(framesize);
	vec4 sb = texture(scenebeauty, uv);
	float sd = texture(scenedepth, uv).r;

	if(gl_FragCoord.z > sd) discard;

	out_color = .66f*sb + .33f*vec4(0,0,1,1);
}