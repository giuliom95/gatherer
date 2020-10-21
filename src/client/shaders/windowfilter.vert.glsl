#version 460 core

uniform mat4 vpmat;
uniform mat4 objmat;
uniform vec2 size;

void main()
{
	uint i = gl_VertexID;
	vec2 uv = vec2((i & 0x1), (i & 0x2) >> 1);
	gl_Position = vec4(0, .5*size*(uv*2 - 1), 1) * objmat * vpmat;
}