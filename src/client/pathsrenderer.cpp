#include "pathsrenderer.hpp"

void PathsRenderer::init()
{
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	glGenBuffers(1, &vboidx);
	glEnableVertexAttribArray(0);
	LOG(info) << "Created VAO";
	
	selectedpaths = std::set<unsigned>();

	shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/paths.vert.glsl",
		"../src/client/shaders/paths.frag.glsl"
	);

	enabledepth = true;

	locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");

	pathsalpha = PATHSRENDERER_DEFPATHSALPHA;
	locid_pathsalpha = glGetUniformLocation(shaprog_idx, "pathsalpha");
	locid_enabledepth = glGetUniformLocation(shaprog_idx, "enabledepth");
	locid_scenedepth = glGetUniformLocation(shaprog_idx, "scenedepth");
	locid_framesize = glGetUniformLocation(shaprog_idx, "framesize");
}

void PathsRenderer::render(Camera& cam, GLuint scenedepthtex, Vec2i framesize)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid_scenedepth, 0);

	glUniform2i(locid_framesize, framesize[0], framesize[1]);

	glMultiDrawArrays(
		GL_LINE_STRIP, 
		paths_firsts.data(),
		paths_lengths.data(),
		selectedpaths.size()
	);
}

void PathsRenderer::addpaths(std::set<unsigned>& paths)
{
	selectedpaths.insert(paths.begin(), paths.end());
}

void PathsRenderer::removepaths(std::set<unsigned>& paths)
{
	for(unsigned p : paths)
		selectedpaths.erase(p);
}

void PathsRenderer::updaterenderlist(GatheredData& gd)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboidx);

	const unsigned npaths = selectedpaths.size();

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
	for(unsigned pi : selectedpaths)
	{
		const unsigned firstbounce = gd.firstbounceindexes[pi];
		const unsigned len = gd.pathlengths[pi];
		paths_firsts[i] = off;
		paths_lengths[i] = len;
		offset_ptrs[i] = &(gd.bouncepositions[firstbounce]);
		//LOG(info) << firstbounce << " " << off << " " << len;
		off += len;
		i++;
	}

	unsigned totalbytes = off * sizeof(Vec3h);
	glBufferData(GL_ARRAY_BUFFER, totalbytes, NULL,  GL_DYNAMIC_DRAW);

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
}

void PathsRenderer::clearpaths()
{
	selectedpaths.clear();
}