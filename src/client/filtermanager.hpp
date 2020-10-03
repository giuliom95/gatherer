#ifndef _FILTERMANAGER_HPP_
#define _FILTERMANAGER_HPP_

#include <list>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "math.hpp"
#include "utils.hpp"
#include "camera.hpp"
#include "gathereddata.hpp"

class Filter
{
public:
	virtual void render(
		Camera& cam,
		GLuint scenefbo_id,
		GLuint scenedepthtex,
		GLuint scenebeautytex,
		Vec2i framesize
	) = 0;

	virtual void setframesize(Vec2i size) = 0;

	virtual void computepaths(GatheredData& gd) = 0;
};

class FilterManager
{
public:
	std::list<std::shared_ptr<Filter>> filterslist;

	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex,
		Vec2i framesize
	);

	void setframesize(Vec2i size);

	void renderui();

	void computepaths(GatheredData& gd);
};

#endif