#version 460 core
in vec2 uv;

out vec4 out_color;

uniform sampler2D opaquebeauty;
uniform sampler2D transbeauty;
uniform sampler2D above;
uniform sampler2D below;

void main()
{
	vec4 ob = texture(opaquebeauty, uv);
	vec4 b = texture(below, uv);
	vec4 tb = texture(transbeauty, uv);
	vec4 a = texture(above, uv);

	vec3 c = ob.rgb;
	c =  (b.r)*vec3(1) + (1 -  b.r)*c;
	c = (tb.a)*tb.rgb  + (1 - tb.a)*c;
	c =  (a.r)*vec3(1) + (1 -  a.r)*c;

	out_color = vec4(c, 1);
}