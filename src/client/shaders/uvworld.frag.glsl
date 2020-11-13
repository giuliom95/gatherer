#version 460 core
in vec3 worldpos;
in flat int uvset;

uniform int currentuvset;

layout(location = 0) out vec3 out_set;

void main()
{
	if(uvset != currentuvset) discard;
	out_set = worldpos;
}