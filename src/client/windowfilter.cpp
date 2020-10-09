#include "windowfilter.hpp"

WindowFilter::WindowFilter()
{
	shaprog_id = disk_load_shader_program(
		"../src/client/shaders/windowfilter.vert.glsl",
		"../src/client/shaders/windowfilter.frag.glsl"
	);

	locid_camvpmat    = glGetUniformLocation(shaprog_id, "vpmat");
	locid_scenedepth  = glGetUniformLocation(shaprog_id, "scenedepth");
	locid_framesize   = glGetUniformLocation(shaprog_id, "framesize");
	locid_scenebeauty = glGetUniformLocation(shaprog_id, "scenebeautytex");
}

void WindowFilter::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex
) {
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	glUseProgram(shaprog_id);
	glBindFramebuffer(GL_FRAMEBUFFER, scenefbo_id);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform2i(locid_framesize, framesize[0], framesize[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid_scenedepth, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scenebeautytex);
	glUniform1i(locid_scenebeauty, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void WindowFilter::computepaths(GatheredData& gd)
{
	// Do nothing for now
	auto npaths = gd.selectedpaths.size();
	LOG(info) << "Filtered paths (" << npaths << " / " << npaths << ")";
}

bool WindowFilter::renderstackui()
{
	bool modified = false;

	modified |= ImGui::DragFloat3(
		"Position", reinterpret_cast<float*>(&position)
	);
	modified |= ImGui::DragFloat3(
		"Normal", reinterpret_cast<float*>(&normal)
	);
	modified |= ImGui::DragFloat2(
		"Size", reinterpret_cast<float*>(&size)
	);
	
	return modified;
}