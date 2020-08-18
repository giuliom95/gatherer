#version 450 core
out vec4 out_color;

uniform float pathsalpha;
uniform sampler2D scenedepth;

void main()
{
	vec2 uv = gl_FragCoord.xy / 1024;
	float sd = texture(scenedepth, uv).r;
	if(gl_FragCoord.z > sd) discard;
	out_color = vec4(vec3(1), pathsalpha);
}