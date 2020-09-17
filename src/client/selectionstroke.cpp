#include "selectionstroke.hpp"

void SelectionStroke::init()
{
	shaprog1_id = disk_load_shader_program(
		"../src/client/shaders/selectionvolume1.vert.glsl",
		"../src/client/shaders/selectionvolume1.frag.glsl",
		"../src/client/shaders/selectionvolume1.tes.glsl"
	);

	locid1_camvpmat   = glGetUniformLocation(shaprog1_id, "vpmat");
	locid1_radius     = glGetUniformLocation(shaprog1_id, "radius");
	locid1_location   = glGetUniformLocation(shaprog1_id, "location");
	locid1_scenedepth = glGetUniformLocation(shaprog1_id, "scenedepth");
	locid1_framesize  = glGetUniformLocation(shaprog1_id, "framesize");

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenTextures(1, &texid_fbomask);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fbomask, 0
	);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaprog2_id = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/selectionvolume2.frag.glsl"
	);

	locid2_scenebeauty	= glGetUniformLocation(shaprog2_id, "scenebeautytex");
	locid2_mask 		= glGetUniformLocation(shaprog2_id, "masktex");

	brushsize = SELECTIONSTROKE_DEFBRUSHSIZE;
}

void SelectionStroke::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex,
	Vec2i framesize
) {
	glDisable(GL_CULL_FACE);

	glUseProgram(shaprog1_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glClear(GL_COLOR_BUFFER_BIT);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	const GLfloat il[]{6,6}; const GLfloat ol[]{6,6,6,6};
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, il);
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, ol);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform2i(locid1_framesize, framesize[0], framesize[1]);

	for(Sphere s : spheres)
	{
		glUniform1f(locid1_radius, s.radius);

		glUniform3f(
			locid1_location,
			s.center[0], s.center[1], s.center[2]
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, scenedepthtex);
		glUniform1i(locid1_scenedepth, 0);

		glDrawArrays(GL_PATCHES, 0, 24);
	}


	glUseProgram(shaprog2_id);
	glBindFramebuffer(GL_FRAMEBUFFER, scenefbo_id);
	glDepthMask(GL_FALSE);

	GLenum buf[]{GL_COLOR_ATTACHMENT1};
	glDrawBuffers(1, buf);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenebeautytex);
	glUniform1i(locid2_scenebeauty, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glUniform1i(locid2_mask, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void SelectionStroke::setframesize(Vec2i size)
{
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED, 
		size[0], size[1], 0, 
		GL_RED, GL_UNSIGNED_BYTE, nullptr
	);
}

void SelectionStroke::addpoint(Vec3f pt)
{
	Sphere s{pt, brushsize};
	spheres.push_back(s);
}

void SelectionStroke::findbounces(GatheredData& gd)
{
	/*
	 * This works fine as it is but, if multithreading has to be implemented,
	 * it can be done with the gd.firstbounceindexes vector.
	 */ 
	unsigned npaths = gd.pathslength.size();
	unsigned off = 0;
	for(unsigned path_i = 0; path_i < npaths; ++path_i)
	{
		const unsigned nbounces = gd.pathslength[path_i];
		for(unsigned b_i = off; b_i < off + nbounces; b_i++)
		{
			Vec3h bounceh = gd.bouncesposition[b_i];
			Vec3f bouncef = fromVec3h(bounceh);
			for(Sphere& s : spheres)
			{
				float d = length(bouncef - s.center);
				if(d <= s.radius)
				{
					selectedpaths.insert(path_i);
					break;
				}
			}
		}
		off += nbounces;
	}
}

void SelectionStroke::clearpoints()
{
	spheres.clear();
	selectedpaths.clear();
}