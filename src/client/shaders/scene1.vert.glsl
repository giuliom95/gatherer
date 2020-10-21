#version 460 core
layout (location = 0) in vec3 vtx_in;

out vec3 worldpos;

uniform mat4 vpmat;

void main()
{
	gl_Position = vec4(vtx_in, 1)*vpmat;
	worldpos = vtx_in;
}