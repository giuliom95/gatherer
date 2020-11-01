#version 460 core
in flat int pathID;

out vec4 out_color;

uniform float pathsalpha;
uniform bool enabledepth;
uniform bool enableradiance;
uniform sampler2D scenedepth;
uniform ivec2 framesize;

layout (binding = 0) buffer RadianceBlock
{
  float radiances[];
}; 

void main()
{
	float pathradiance = radiances[pathID];
	if(enableradiance && pathradiance*pathsalpha < 0.0001) discard;

	vec2 uv = gl_FragCoord.xy / vec2(framesize);
	float sd = texture(scenedepth, uv).r;
	if(enabledepth && gl_FragCoord.z > sd) discard;

	float a = enableradiance ? pathradiance : 1;
	out_color = vec4(vec3(1), min(a*pathsalpha, 1));
}