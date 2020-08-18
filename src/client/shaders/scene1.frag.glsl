#version 450 core
in vec3 viewp_frag;

out vec4 out_color;

uniform vec3 color;

void main()
{
	vec3 xt = dFdx(viewp_frag);
    vec3 yt = dFdy(viewp_frag);
    vec3 n = normalize(cross(xt, yt));
	vec3 l = normalize(viewp_frag);
	vec3 diff = (0.5 + 0.5*abs(dot(n,l))) * color;
	out_color = vec4(diff, 1);
}