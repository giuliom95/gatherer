#version 450 core
in vec2 uv;

out vec4 out_color;

uniform mat4  vpmat;
uniform mat4  plane2w;
uniform float radius;
uniform sampler2D scenedepth;

void main() 
{
	vec2 uvc = 2*uv - 1;

	vec2 uvc2 = uvc*uvc;
	float d = sqrt(uvc2.x + uvc2.y);
	if(d > 1) discard;
	float z = sqrt(1 - d*d);

	vec3 lpos = radius*vec3(uvc,z);

	/*
	vec4 wpos = vec4(lpos, 1)*plane2w;
	vec4 ndcpos = wpos*vpmat;
	ndcpos /= ndcpos.w;
	*/

	vec2 screenuv = gl_FragCoord.xy / 1024;
	float sd = texture(scenedepth, screenuv).r;
	if(gl_FragCoord.z - z/(1999) > sd) discard;

	out_color = vec4(uv,0,1);

}