#version 450 core
in vec3 n_frag;
in vec3 p_frag;

out vec4 out_color;

uniform vec3 color;
uniform vec3 eye;

void main()
{
	vec3 l = normalize(eye - p_frag);
	vec3 diff = (0.2 + 0.8*abs(dot(n_frag, l))) * color;
	out_color = vec4(diff, 1);
}