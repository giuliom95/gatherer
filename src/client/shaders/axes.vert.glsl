#version 460 core
flat out uint axis;

void main()
{
	uint i = gl_VertexID;
	axis = i / 2;
	gl_Position = vec4(
		(i & 0x001) & uint(axis == 0),
		(i & 0x001) & uint(axis == 1),
		(i & 0x001) & uint(axis == 2),
		1
	);
}