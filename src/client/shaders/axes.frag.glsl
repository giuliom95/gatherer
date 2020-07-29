#version 450 core
flat in uint axis;
out vec4 out_color;

void main()
{
	out_color = vec4(
		float(axis == 0), 
		float(axis == 1), 
		float(axis == 2), 
		1
	);
}