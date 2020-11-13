#version 460 core
layout (location = 0) in vec3 vtx_in;
layout (location = 1) in vec2 uv_in;

out vec3 worldpos;
out int uvset;

void main()
{
	uvset = int(floor(uv_in[0]));
	gl_Position = vec4(
		2*mod(uv_in[0], 1)-1, 
		2*uv_in[1]-1, 
		0, 1
	);
	worldpos = vtx_in;
	
}