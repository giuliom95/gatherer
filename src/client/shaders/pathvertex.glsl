#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;

void main()
{
	//mat4 m = mat4(
	//	-4.37444448471,       0.0,             1.22467117936e-16, -1.22464679915e-16, 
	//	 0.0,                 4.37444448471,   0.0,                0.0, 
	//	-5.35714943625e-16,   0.0,            -1.00001990795,      1.0, 
	//	 1216.09556675,      -1194.22334433,  -1186.37361909,      1186.54999929
	//);
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0)*view;
	gl_Position /= gl_Position.w;
}