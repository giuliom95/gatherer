#version 450 core
layout (location = 0) in vec3 vtx_in;

uniform mat4 vpmat;

void main()
{
	gl_Position = vec4(vtx_in.x, vtx_in.y, vtx_in.z, 1.0)*vpmat;
}