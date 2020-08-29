#version 450 core
out vec2 uv;

void main()
{
	uint i = gl_VertexID;
	uv = vec2((i & 0x1), (i & 0x2) >> 1);
	gl_Position = vec4(uv*2 - 1, 0, 1);
}