#version 450 core
out vec2 uv;

uniform mat4  vpmat;
uniform mat4  plane2w;
uniform float radius;

void main()
{
	uint i = gl_VertexID;
	uv = vec2((i & 0x2) >> 1, (i & 0x1));

	gl_Position = vec4(radius*(uv*2 - 1), 0, 1)*plane2w*vpmat;
}