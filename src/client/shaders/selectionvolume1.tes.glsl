#version 450 core
layout(quads) in;

uniform float radius;
uniform mat4 vpmat;

void main() 
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	//bilinear interpolation
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec4 p3 = gl_in[3].gl_Position;
	vec4 a = p0*(1-v) + p1*v;
	vec4 b = p3*(1-v) + p2*v;
	vec3 p = (a*(1-u) + b*u).xyz;

	gl_Position.xyz = radius*normalize(p);
	gl_Position.w = 1;
	gl_Position *= vpmat;
}

