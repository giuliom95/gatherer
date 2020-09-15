#include "imagerenderer.hpp"

void ImageRenderer::init(GatheredData& gd)
{
	rendersize = gd.rendersize;

	shaprog_id = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/image.frag.glsl"
	); 

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenTextures(1, &fbotex_id);
	glBindTexture(GL_TEXTURE_2D, fbotex_id);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB,
		rendersize[0], rendersize[1],
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, fbotex_id, 0
	);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Gather luminance into texture
	renderedimage = std::vector<Vec3h>(rendersize[0]*rendersize[1]);
	//std::vector<unsigned> spp(rendersize[0]*rendersize[1], 0);
	for(unsigned pi = 0; pi < gd.pathscamerasamples.size(); ++pi)
	{
		Vec2h& sample = gd.pathscamerasamples[pi];
		Vec2i ppos{
			(int)(rendersize[0]*sample[0]),
			(int)(rendersize[1]*sample[1])
		};
		if(ppos[0] == rendersize[0]) ppos[0]--;
		if(ppos[1] == rendersize[1]) ppos[1]--;
		
		const unsigned idx = ppos[0] + rendersize[0]*ppos[1];
		Vec3h& p = renderedimage[idx];
		p = p + gd.pathsluminance[pi];
		//spp[idx]++;
	}
	LOG(info) << "Accumulated paths";
	for(unsigned idx = 0; idx < renderedimage.size(); idx++)
	{
		Vec3h& p = renderedimage[idx];
		////LOG(info) << p;
		//p = (half)(1.0f / spp[idx]) * p;
		////LOG(info) << spp[idx] << " " << p;
		p = p / (half)gd.rendersamples;
	}

	glGenTextures(1, &renderedimagetex_id);
	glBindTexture(GL_TEXTURE_2D, renderedimagetex_id);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB16F, 
		rendersize[0], rendersize[1], 
		0, GL_RGB,  GL_HALF_FLOAT, renderedimage.data() 
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	locid_renderedimagetex = 
		glGetUniformLocation(shaprog_id, "renderedimagetex");

	exposure = 0;
	locid_exposure = glGetUniformLocation(shaprog_id, "exposure");
}

void ImageRenderer::render()
{
	glUseProgram(shaprog_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glViewport(0, 0, rendersize[0], rendersize[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedimagetex_id);
	glUniform1i(locid_renderedimagetex, 0);

	glUniform1f(locid_exposure, exposure);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}