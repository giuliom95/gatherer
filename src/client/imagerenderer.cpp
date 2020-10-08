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
	LOG(info) << "Final render size: " << rendersize;
	renderedimage = std::vector<Vec3h>(rendersize[0]*rendersize[1]);
	for(unsigned pi = 0; pi < gd.pathscamerasamples.size(); ++pi)
	{
		CameraSample& s = gd.pathscamerasamples[pi];
		const unsigned idx = s.i + rendersize[0]*s.j;
		Vec3h& p = renderedimage[idx];
		p = p + gd.pathsluminance[pi];
	}
	LOG(info) << "Accumulated paths";
	for(unsigned idx = 0; idx < renderedimage.size(); idx++)
	{
		Vec3h& p = renderedimage[idx];
		p = p / (half)gd.rendersamples;
	}

	glGenTextures(1, &renderedimagetex_id);
	glBindTexture(GL_TEXTURE_2D, renderedimagetex_id);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB16F, 
		rendersize[0], rendersize[1], 
		0, GL_RGB,  GL_HALF_FLOAT, renderedimage.data() 
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	pathmask = std::vector<uint16_t>(rendersize[0]*rendersize[1], 255);
	glGenTextures(1, &pathmasktex_id);
	glBindTexture(GL_TEXTURE_2D, pathmasktex_id);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_R16, 
		rendersize[0], rendersize[1], 
		0, GL_RED, GL_UNSIGNED_SHORT, pathmask.data() 
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	locid_renderedimagetex = 
		glGetUniformLocation(shaprog_id, "renderedimagetex");

	locid_pathmasktex = glGetUniformLocation(shaprog_id, "pathmasktex");
	locid_spp = glGetUniformLocation(shaprog_id, "maxpathmask");
	spp_over_2e16 = (float)gd.rendersamples / (1 << 16);

	exposure = 0;
	locid_exposure = glGetUniformLocation(shaprog_id, "exposure");

	bgcolor = Vec3f{1,0,1};
	locid_bgcolor = glGetUniformLocation(shaprog_id, "bgcolor");

	displaymode = finalluminance;
	locid_displaymode = glGetUniformLocation(shaprog_id, "displaymode");
}

void ImageRenderer::updatepathmask(GatheredData& gd)
{
	for(uint16_t& p : pathmask) p = 0;
	for(unsigned pi : gd.selectedpaths)
	{
		CameraSample& cs = gd.pathscamerasamples[pi];
		uint16_t& v = pathmask[cs.i + gd.rendersize[0]*cs.j];
		v++;
	}
	
	glBindTexture(GL_TEXTURE_2D, pathmasktex_id);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, rendersize[0], rendersize[1],
		GL_RED, GL_UNSIGNED_SHORT, pathmask.data()
	);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void ImageRenderer::render()
{
	glUseProgram(shaprog_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glViewport(0, 0, rendersize[0], rendersize[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedimagetex_id);
	glUniform1i(locid_renderedimagetex, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pathmasktex_id);
	glUniform1i(locid_pathmasktex, 1);

	glUniform1f(locid_spp, spp_over_2e16);

	glUniform1f(locid_exposure, exposure);
	glUniform3f(locid_bgcolor, bgcolor[0], bgcolor[1], bgcolor[2]);

	glUniform1i(locid_displaymode, (int)displaymode);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}