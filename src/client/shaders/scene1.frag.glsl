#version 450 core
in vec3 worldpos;

layout(location = 0) out vec4 out_beauty;
layout(location = 1) out vec3 out_worldpos;

uniform vec3 albedo;
uniform vec3 eye;

void main()
{
	vec3 xt = dFdx(worldpos);
	vec3 yt = dFdy(worldpos);
	vec3 n = normalize(cross(xt, yt));
	vec3 l = normalize(worldpos - eye);
	vec3 diff = (0.5 + 0.5*abs(dot(n,l))) * albedo;
	out_beauty = vec4(diff, 1);
	out_worldpos = worldpos;
}