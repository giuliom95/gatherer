#version 450 core
in vec2 uv;

out vec3 out_color;

uniform sampler2D renderedimagetex;
uniform sampler2D pathmasktex;
uniform float exposure;

void main()
{
	float a = pow(2, exposure + 2.47393f);
	float b = 84.66f;
	float invGamma = 1 / 2.2f;
	//vec3 hdr = texelFetch(renderedimagetex, ivec2(floor(gl_FragCoord.xy)), 0).rgb;
	vec3 hdr = texture(renderedimagetex, uv).rgb;
	out_color = clamp(b*pow(a*hdr, vec3(invGamma)), 0.0f, 255.0f) / 255.0f;

	out_color *= texture(pathmasktex, uv).r;
}