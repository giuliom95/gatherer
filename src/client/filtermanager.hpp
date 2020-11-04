#ifndef _FILTERMANAGER_HPP_
#define _FILTERMANAGER_HPP_

#include <list>
#include <string>

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
		GLuint scenebeautytex
	) = 0;

	virtual void setframesize(Vec2i size)
	{
		framesize = size;
	}

	virtual void computepaths(GatheredData& gd) = 0;

	virtual bool renderstackui() = 0;

	std::string filtertypename = "Filter";

	Vec2i framesize;
	unsigned globalid;

	bool visible = true;
};

class FilterManager
{
private:
	std::list<std::shared_ptr<Filter>> filterslist;

	unsigned progressiveid = 0;
public:

	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex
	);

	void addfilter(std::shared_ptr<Filter> filter);

	void setframesize(Vec2i size);

	// Return true if something changed
	bool renderui();

	void computepaths(GatheredData& gd);
};

#endif