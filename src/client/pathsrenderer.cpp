#include "pathsrenderer.hpp"

void PathsRenderer::init()
{
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	glGenBuffers(1, &posvboidx);
	glGenBuffers(1, &colorvboidx);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	LOG(info) << "Created VAO";

	shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/paths.vert.glsl",
		"../src/client/shaders/paths.frag.glsl"
	);

	enablerendering = true;

	enabledepth = true;

	locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");

	pathsalpha = PATHSRENDERER_DEFPATHSALPHA;
	locid_pathsalpha = glGetUniformLocation(shaprog_idx, "pathsalpha");
	locid_enabledepth = glGetUniformLocation(shaprog_idx, "enabledepth");
	locid_enableradiance = glGetUniformLocation(shaprog_idx, "enableradiance");
	locid_scenedepth = glGetUniformLocation(shaprog_idx, "scenedepth");
	locid_framesize = glGetUniformLocation(shaprog_idx, "framesize");
}

void PathsRenderer::render(
	Camera& cam, 
	GLuint fbo,
	GLuint scenedepthtex,
	Vec2i framesize,
	GatheredData& gd
) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glUseProgram(shaprog_idx);
	glBindVertexArray(vaoidx);
	glLineWidth(1);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform1f(locid_pathsalpha, pathsalpha);
	glUniform1i(locid_enabledepth, enabledepth);
	glUniform1i(locid_enableradiance, enableradiance);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid_scenedepth, 0);

	glUniform2i(locid_framesize, framesize[0], framesize[1]);

	glMultiDrawArrays(
		GL_LINE_STRIP, 
		paths_firsts.data(),
		paths_lengths.data(),
		gd.selectedpaths.size()
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

	unsigned off = 0;
	unsigned i = 0;
	for(unsigned pi : gd.selectedpaths)
	{
		const unsigned firstbounce = gd.firstbounceindexes[pi];
		const unsigned len = gd.pathslength[pi];
		paths_firsts[i] = off;
		paths_lengths[i] = len;
		offset_ptrs[i] = &(gd.bouncesposition[firstbounce]);
		//LOG(info) << firstbounce << " " << off << " " << len;
		off += len;
		i++;
	}

	const unsigned nvtx = off;
	std::vector<half> pathsenergy(nvtx);

	const unsigned totalvtxbytes = nvtx * sizeof(Vec3h);
	glBindBuffer(GL_ARRAY_BUFFER, posvboidx);
	glBufferData(GL_ARRAY_BUFFER, totalvtxbytes, NULL,  GL_DYNAMIC_DRAW);

	for(unsigned i = 0; i < npaths; ++i)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			paths_firsts[i] * sizeof(Vec3h), 
			paths_lengths[i] * sizeof(Vec3h), 
			offset_ptrs[i]
		);

		const Vec3h radiance = gd.pathsradiance[i];
		const half pathenergy = radiance[0] + radiance[1] + radiance[2];
		std::fill(
			pathsenergy.begin() + paths_firsts[i], 
			pathsenergy.begin() + paths_firsts[i] + paths_lengths[i],
			pathenergy
		);
	}

	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half), (void*)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, colorvboidx);
	glBufferData(
		GL_ARRAY_BUFFER, 
		nvtx*sizeof(half), pathsenergy.data(),
		GL_DYNAMIC_DRAW
	);
	glVertexAttribPointer(
		1, 1, GL_HALF_FLOAT, 
		GL_FALSE, sizeof(half), (void*)0
	);
}