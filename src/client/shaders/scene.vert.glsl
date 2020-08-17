#version 450 core
layout (location = 0) in vec3 vtx_in;
layout (location = 1) in vec3 n_in;

out vec3 n_frag;
out vec3 p_frag;

uniform mat4 vpmat;

void main()
{
	gl_Position = vec4(vtx_in.x, vtx_in.y, vtx_in.z, 1)*vpmat;
	n_frag = n_in;
	p_frag = vtx_in;
}