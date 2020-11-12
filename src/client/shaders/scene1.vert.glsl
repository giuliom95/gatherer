#version 460 core
layout (location = 0) in vec3 vtx_in;
layout (location = 1) in vec2 uv_in;

out vec3 worldpos;
out vec2 uv;

uniform mat4 vpmat;

void main()
{
	gl_Position = vec4(vtx_in, 1)*vpmat;
	worldpos = vtx_in;
	uv = uv_in;
}