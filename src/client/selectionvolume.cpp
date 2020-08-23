#include "selectionvolume.hpp"

void SelectionVolume::init()
{
	shaprog_id = disk_load_shader_program(
		"../src/client/shaders/selectionvolume.vert.glsl",
		"../src/client/shaders/selectionvolume.frag.glsl",
		"../src/client/shaders/selectionvolume.tes.glsl"
	);

	locid_camvpmat   = glGetUniformLocation(shaprog_id, "vpmat");
	locid_radius     = glGetUniformLocation(shaprog_id, "radius");
	locid_scenedepth = glGetUniformLocation(shaprog_id, "scenedepth");

	location = Vec3f{0,0,0};
	radius = 100;
}

void SelectionVolume::render(Camera& cam, GLuint fbo_id)
{
	glUseProgram(shaprog_id);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	const GLfloat il[]{6,6}; const GLfloat ol[]{6,6,6,6};
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, il);
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, ol);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform1f(locid_radius, radius);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, fbo_id);
	//glUniform1i(locid_scenedepth, 0);

	glDrawArrays(GL_PATCHES, 0, 24);
}