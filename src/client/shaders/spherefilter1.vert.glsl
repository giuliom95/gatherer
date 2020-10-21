#version 460 core

void main()
{
	uint i = gl_VertexID;
	if(i ==  0 || i ==  4 || i == 12) gl_Position = vec4(+1,-1,+1, 1);
	if(i ==  3 || i ==  5 || i ==  8) gl_Position = vec4(+1,-1,-1, 1);
	if(i ==  2 || i ==  9 || i == 19) gl_Position = vec4(-1,-1,-1, 1);
	if(i ==  1 || i == 15 || i == 16) gl_Position = vec4(-1,-1,+1, 1);
	if(i ==  7 || i == 13 || i == 20) gl_Position = vec4(+1,+1,+1, 1);
	if(i ==  6 || i == 11 || i == 21) gl_Position = vec4(+1,+1,-1, 1);
	if(i == 10 || i == 18 || i == 22) gl_Position = vec4(-1,+1,-1, 1);
	if(i == 14 || i == 17 || i == 23) gl_Position = vec4(-1,+1,+1, 1);
}