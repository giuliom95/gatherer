#include "selectionvolume.hpp"

void SelectionVolume::init()
{
	shaprog_id = disk_load_shader_program(
		"../src/client/shaders/selectionvolume.vert.glsl",
		"../src/client/shaders/selectionvolume.frag.glsl"
	);

	locid_camvpmat   = glGetUniformLocation(shaprog_id, "vpmat");
	locid_plane2w    = glGetUniformLocation(shaprog_id, "plane2w");
	locid_radius     = glGetUniformLocation(shaprog_id, "radius");
	locid_scenedepth = glGetUniformLocation(shaprog_id, "scenedepth");

	location = Vec3f{0,0,0};
	radius = 100;
}

void SelectionVolume::render(Camera& cam, GLuint texid_scenedepth)
{
	glUseProgram(shaprog_id);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	const Vec3f n = transformVector(cam.c2w(), Vec3f{0,0,-1});
	const Vec3f u = normalize(cross(n, Vec3f{0,1,0}));
	const Vec3f v = cross(n,  u);
	const Mat4f plane2w(u, v, n, location);
	glUniformMatrix4fv(
		locid_plane2w, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&plane2w)
	);

	glUniform1f(locid_radius, radius);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid_scenedepth);
	glUniform1i(locid_scenedepth, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}