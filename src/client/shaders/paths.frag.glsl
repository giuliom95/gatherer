#version 450 core
out vec4 out_color;

uniform float pathsalpha;
uniform bool enabledepth;
uniform sampler2D scenedepth;
uniform ivec2 framesize;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(framesize);
	float sd = texture(scenedepth, uv).r;
	if(enabledepth && gl_FragCoord.z > sd) discard;
	out_color = vec4(vec3(1), pathsalpha);
}