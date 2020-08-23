#include "selectionvolume.hpp"

void SelectionVolume::init()
{
	shaprog1_id = disk_load_shader_program(
		"../src/client/shaders/selectionvolume1.vert.glsl",
		"../src/client/shaders/selectionvolume1.frag.glsl",
		"../src/client/shaders/selectionvolume1.tes.glsl"
	);

	locid1_camvpmat   = glGetUniformLocation(shaprog1_id, "vpmat");
	locid1_radius     = glGetUniformLocation(shaprog1_id, "radius");
	locid1_scenedepth = glGetUniformLocation(shaprog1_id, "scenedepth");

	location = Vec3f{0,0,0};
	radius = 100;

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenTextures(1, &texid_fbomask);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED, 
		WINDOW_W, WINDOW_H, 0, 
		GL_RED, GL_UNSIGNED_BYTE, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fbomask, 0
	);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		BOOST_LOG_TRIVIAL(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaprog2_id = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/selectionvolume2.frag.glsl"
	);

	locid2_scenebeauty	= glGetUniformLocation(shaprog2_id, "scenebeautytex");
	locid2_mask 		= glGetUniformLocation(shaprog2_id, "masktex");
}

void SelectionVolume::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex
) {
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

	glUniform1f(locid1_radius, radius);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid1_scenedepth, 0);

	glDrawArrays(GL_PATCHES, 0, 24);


	glUseProgram(shaprog2_id);
	glBindFramebuffer(GL_FRAMEBUFFER, scenefbo_id);
	glDepthMask(GL_FALSE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenebeautytex);
	glUniform1i(locid2_scenebeauty, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glUniform1i(locid2_mask, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDepthMask(GL_TRUE);
}