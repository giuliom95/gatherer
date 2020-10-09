#ifndef _WINDOWFILTER_HPP_
#define _WINDOWFILTER_HPP_

#include "math.h"
#include "filtermanager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

class WindowFilter : public Filter
{
	WindowFilter();

	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex
	) override;

	void computepaths(GatheredData& gd) override;

	bool renderstackui() override;

	Mat4f w2o, o2w;
	Vec3f position, normal;
	Vec2f size;

private:

	GLuint shaprog_id;
	GLuint locid_camvpmat;
	GLuint locid_scenedepth;
	GLuint locid_framesize;
	GLuint locid_scenebeauty;

};

#endif