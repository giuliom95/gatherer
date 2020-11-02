#version 460 core
in flat int pathID;

out vec4 out_above;
out vec4 out_below;

uniform float pathsalpha;
uniform bool enabledepth;
uniform bool enableradiance;

uniform sampler2D opaquedepth;
uniform sampler2D transdepth;

layout (binding = 0) buffer RadianceBlock
{
  float radiances[];
}; 

void main()
{
	float pathradiance = radiances[pathID];
	if(enableradiance && pathradiance*pathsalpha < 0.0001) discard;

	ivec2 pixcoords = ivec2(gl_FragCoord.xy);
	float d = gl_FragCoord.z;
	float od = texelFetch(opaquedepth, pixcoords, 0).r;
	if(enabledepth && d > od) discard;

	// Path color
	float pam = enableradiance ? pathradiance : 1;
	float pa = min(pam*pathsalpha, 1);
	//vec4 pc = vec4(vec3(1), pa);

	float td = texelFetch(transdepth, pixcoords, 0).r;
	if(d > td)
	{
		/*vec3 ob = texelFetch(opaquebeauty, pixcoords, 0).rgb;
		vec4 tb = texelFetcvec4(1,0,0,h(transbeauty, pixcoords, 0);
		
		// Color of ray on opaque
		vec3 c1 = (1-pc.a)*ob + pc.a*pc.rgb;

		out_color.rgb = tb.a*tb.rgb + (1-tb.a)*c1;
		out_color.a = 1;
		*/
		out_below = vec4(1,1,1,pa);
	}
	else
	{
		// Unoccluded ray
		//out_color = pc;
		out_above = vec4(1,1,1,pa);
	}

}