#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 vpmat;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0)*vpmat;
	gl_Position.w += .42;
}