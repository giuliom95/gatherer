#version 450 core
in vec3 n_frag;

out vec4 out_color;

uniform vec3 color;

void main()
{
	vec3 l = vec3(0.707106781, 0.707106781, 0);
	vec3 d = dot(l, n_frag)*color;
	out_color = vec4(d, 1);
	
}