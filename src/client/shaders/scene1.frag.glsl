#version 460 core
in vec3 worldpos;

layout(location = 0) out vec3 out_worldpos;
layout(location = 1) out vec4 out_beauty;

uniform vec3  color;
uniform float alpha;

uniform vec3 eye;

uniform vec4 blend;

void main()
{
	vec3 xt = dFdx(worldpos);
	vec3 yt = dFdy(worldpos);
	vec3 n = normalize(cross(xt, yt));
	vec3 l = normalize(worldpos - eye);
	vec3 diff = (0.5 + 0.5*abs(dot(n,l))) * color ;
	diff = (1-blend.a) * diff + blend.a*blend.rgb;
	out_beauty = vec4(diff, 1);
	out_worldpos = worldpos;
}