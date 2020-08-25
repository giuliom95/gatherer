#version 460 core
flat out uint axis;

uniform mat4 vpmat;

void main()
{
	uint i = gl_VertexID;
	axis = i / 2;
	gl_Position = vec4(
		(i & 0x001) & uint(axis == 0),
		(i & 0x001) & uint(axis == 1),
		(i & 0x001) & uint(axis == 2),
		0
	);
	gl_Position = gl_Position*vpmat;
	gl_Position.z *= -1;
	gl_Position.w = 1;
}