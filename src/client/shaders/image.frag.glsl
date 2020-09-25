#version 450 core
in vec2 uv;

out vec3 out_color;

uniform sampler2D renderedimagetex;
uniform sampler2D pathmasktex;
uniform float maxpathmask;
uniform float exposure;
uniform vec3 bgcolor;
uniform int displaymode;

void main()
{

	float pathmask = texture(pathmasktex, uv).r;

	// luminance
	if(displaymode == 0)
	{
		if(pathmask == 0)
			out_color = bgcolor;
		else
		{
			float a = pow(2, exposure + 2.47393f);
			float b = 84.66f;
			float invGamma = 1 / 2.2f;
			vec3 hdr = texture(renderedimagetex, uv).rgb;
			out_color = clamp(b*pow(a*hdr, vec3(invGamma)), 0.0f, 255.0f) / 255.0f;
		}
	}
	// bounce map, paths per pixel
	else if(displaymode == 1)
	{
		if(pathmask == 0)
			out_color = bgcolor;
		else
		{
			float x = pathmask / maxpathmask;
			// Coolwarm mapping
			vec3 low = vec3(0.230, 0.299, 0.754);
			vec3 mid = vec3(0.865, 0.865, 0.865);
			vec3 hig = vec3(0.706, 0.016, 0.150);
			float s1 = step( x,  .5);
			float s2 = step(-x, -.5);
			float y = 2*x;
			float z = y - 1;
			vec3 a = (1-y)*low + y*mid;
			vec3 b = (1-z)*mid + z*hig;
			out_color = s1*a + s2*b;
		}
	}
}