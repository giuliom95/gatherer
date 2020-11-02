#include "pathsrenderer.hpp"

void PathsRenderer::init()
{
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	glGenBuffers(1, &posvboidx);
	glGenBuffers(1, &ssboidx);
	glEnableVertexAttribArray(0);
	LOG(info) << "Created VAO";

	enablerendering = true;
	enabledepth = true;
	pathsalpha = PATHSRENDERER_DEFPATHSALPHA;

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glGenTextures(1, &texid_above);
	glBindTexture(GL_TEXTURE_2D, texid_above);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &texid_below);
	glBindTexture(GL_TEXTURE_2D, texid_below);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_above, 0
	);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, texid_below, 0
	);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Paths framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	shaprog1_idx = disk_load_shader_program(
		"../src/client/shaders/paths1.vert.glsl",
		"../src/client/shaders/paths1.frag.glsl"
	);
	locid1_camvpmat = glGetUniformLocation(shaprog1_idx, "vpmat");
	locid1_pathsalpha = glGetUniformLocation(shaprog1_idx, "pathsalpha");
	locid1_enabledepth = glGetUniformLocation(shaprog1_idx, "enabledepth");
	locid1_enableradiance = glGetUniformLocation(shaprog1_idx, "enableradiance");
	locid1_opaquedepth = glGetUniformLocation(shaprog1_idx, "opaquedepth");
	locid1_transdepth = glGetUniformLocation(shaprog1_idx, "transdepth");

	shaprog2_idx = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/paths2.frag.glsl"
	);
	locid2_opaquebeauty = glGetUniformLocation(shaprog2_idx, "opaquebeauty");
	locid2_transbeauty = glGetUniformLocation(shaprog2_idx, "transbeauty");
	locid2_above = glGetUniformLocation(shaprog2_idx, "above");
	locid2_below = glGetUniformLocation(shaprog2_idx, "below");
}

void PathsRenderer::render(
	Camera& cam, 
	GLuint finalfbo,
	GLuint opaquebeautytex,
	GLuint transbeautytex,
	GLuint opaquedepthtex,
	GLuint transdepthtex,
	GatheredData& gd
) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	GLenum buf1[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, buf1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(shaprog1_idx);
	glBindVertexArray(vaoidx);
	glLineWidth(1);
	//glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_SRC_COLOR);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform1f(locid1_pathsalpha, pathsalpha);
	glUniform1i(locid1_enabledepth, enabledepth);
	glUniform1i(locid1_enableradiance, enableradiance);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, opaquedepthtex);
	glUniform1i(locid1_opaquedepth, 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, transdepthtex);
	glUniform1i(locid1_transdepth, 1);

	glMultiDrawArrays(
		GL_LINE_STRIP, 
		paths_firsts.data(),
		paths_lengths.data(),
		gd.selectedpaths.size()
	);

	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glBindFramebuffer(GL_FRAMEBUFFER, finalfbo);
	GLenum buf2[]{GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buf2);

	glUseProgram(shaprog2_idx);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, opaquebeautytex);
	glUniform1i(locid2_opaquebeauty, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, transbeautytex);
	glUniform1i(locid2_transbeauty, 1);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texid_above);
	glUniform1i(locid2_above, 2);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texid_below);
	glUniform1i(locid2_below, 3);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

void PathsRenderer::setframesize(Vec2i size)
{
	glBindTexture(GL_TEXTURE_2D, texid_above);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA16F, 
		size[0], size[1], 0, 
		GL_RGBA, GL_HALF_FLOAT, nullptr
	);
	glBindTexture(GL_TEXTURE_2D, texid_below);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA16F, 
		size[0], size[1], 0, 
		GL_RGBA, GL_HALF_FLOAT, nullptr
	);
}

void PathsRenderer::updaterenderlist(GatheredData& gd)
{
	glBindVertexArray(vaoidx);
	const unsigned npaths = gd.selectedpaths.size();

	paths_firsts  = std::vector<GLint>(npaths);
	paths_lengths = std::vector<GLsizei>(npaths);

	if(npaths == 0)
	{
		glBufferData(GL_ARRAY_BUFFER, 0, NULL,  GL_DYNAMIC_DRAW);
		return;
	}

	std::vector<Vec3h*> offset_ptrs(npaths);
	std::vector<float> pathsenergy(npaths);

	unsigned off = 0;
	unsigned i = 0;
	for(unsigned pi : gd.selectedpaths)
	{
		const unsigned firstbounce = gd.firstbounceindexes[pi];
		const unsigned len = gd.pathslength[pi];
		paths_firsts[i] = off;
		paths_lengths[i] = len;
		offset_ptrs[i] = &(gd.bouncesposition[firstbounce]);

		const Vec3h radiance = gd.pathsradiance[pi];
		const float pathenergy = radiance[0] + radiance[1] + radiance[2];
		pathsenergy[i] = pathenergy;

		off += len;
		i++;
	}

	const unsigned nvtx = off;

	const unsigned totalvtxbytes = nvtx * sizeof(Vec3h);
	glBindBuffer(GL_ARRAY_BUFFER, posvboidx);
	glBufferData(GL_ARRAY_BUFFER, totalvtxbytes, NULL,  GL_STATIC_DRAW);

	for(unsigned i = 0; i < npaths; ++i)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			paths_firsts[i] * sizeof(Vec3h), 
			paths_lengths[i] * sizeof(Vec3h), 
			offset_ptrs[i]
		);
	}

	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half), (void*)0
	);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboidx);
	glBufferData(
		GL_SHADER_STORAGE_BUFFER, 
		sizeof(float)*npaths, 
		pathsenergy.data(),
		GL_STATIC_DRAW
	);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboidx);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}