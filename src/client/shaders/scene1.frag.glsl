#version 460 core
in vec3 worldpos;
in vec2 uv;

layout(location = 0) out vec4 out_worldposid;
layout(location = 1) out vec4 out_beauty;

uniform vec3  color;
uniform float alpha;

uniform vec3 eye;

uniform vec4 blend;

uniform sampler2D opaquedepth;

uniform int geomid;
uniform bool highlight;

void main()
{
	if(alpha < 1)
	{
		float sd = texelFetch(opaquedepth, ivec2(gl_FragCoord.xy), 0).r;
		if(gl_FragCoord.z > sd) discard;
	}

	vec3 xt = dFdx(worldpos);
	vec3 yt = dFdy(worldpos);
	vec3 n = normalize(cross(xt, yt));
	vec3 l = normalize(worldpos - eye);
	//vec3 diff = (0.5 + 0.5*abs(dot(n,l))) * color;
	vec3 diff = vec3(mod(uv[0], 1), uv[1], 0);
	diff = (1-blend.a) * diff + blend.a*blend.rgb;
	if(
		highlight && 
		int(gl_FragCoord.x) % 2 == 0 && 
		int(gl_FragCoord.y) % 2 == 0 
	) {
		diff = vec3(1,1,0);
	}
	out_beauty = vec4(diff, alpha);
	out_worldposid.rgb = worldpos;
	out_worldposid.a = geomid;
}