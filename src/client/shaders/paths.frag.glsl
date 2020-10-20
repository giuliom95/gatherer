#version 450 core
in float lum_frag;

out vec4 out_color;

uniform float pathsalpha;
uniform bool enabledepth;
uniform bool enableradiance;
uniform sampler2D scenedepth;
uniform ivec2 framesize;

void main()
{
	if(enableradiance && lum_frag*pathsalpha < 0.0001) discard;
	vec2 uv = gl_FragCoord.xy / vec2(framesize);
	float sd = texture(scenedepth, uv).r;
	if(enabledepth && gl_FragCoord.z > sd) discard;
	float fm = enableradiance ? lum_frag : 1;
	out_color = vec4(vec3(1), fm*pathsalpha);
}