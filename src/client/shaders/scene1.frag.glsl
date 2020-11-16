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

uniform sampler2D uvworld0;
uniform sampler2D uvworld1;
uniform sampler2D uvworld2;

vec3 coolwarm(float x)
{
	vec3 low = vec3(0.230, 0.299, 0.754);
	vec3 mid = vec3(0.865, 0.865, 0.865);
	vec3 hig = vec3(0.706, 0.016, 0.150);
	float s1 = step( x,  .5);
	float s2 = step(-x, -.5);
	float y = 2*x;
	float z = y - 1;
	vec3 a = (1-y)*low + y*mid;
	vec3 b = (1-z)*mid + z*hig;
	return s1*a + s2*b;
}


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
	//vec3 diff = vec3(mod(uv[0], 1), uv[1], 0);

	int uvset = int(floor(uv[0]));
	float x = 0;
	if(uvset == 0) x = texture(uvworld0, vec2(mod(uv[0], 1), uv[1])).r;
	if(uvset == 1) x = texture(uvworld1, vec2(mod(uv[0], 1), uv[1])).r;
	if(uvset == 2) x = texture(uvworld2, vec2(mod(uv[0], 1), uv[1])).r;
	vec3 diff = vec3(x,0,0);

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