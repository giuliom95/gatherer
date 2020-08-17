#version 450 core
out vec4 out_color;

uniform vec3 color;

void main()
{
	out_color = vec4(color.r, color.g, color.b, 1);

	// HACK. Must fix persp 
	//gl_FragDepth = 1 - gl_FragCoord.z;
}