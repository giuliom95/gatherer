#ifndef _FILTERMANAGER_HPP_
#define _FILTERMANAGER_HPP_

#include <list>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "math.hpp"
#include "camera.hpp"

class Filter
{
public:
	virtual void render(
		Camera& cam,
		GLuint scenefbo_id,
		GLuint scenedepthtex,
		GLuint scenebeautytex,
		Vec2i framesize
	) {
		(void)cam;
		(void)scenefbo_id;
		(void)scenedepthtex;
		(void)scenebeautytex;
		(void)framesize;
	};
};

class FilterManager
{
public:
	std::list<Filter> filterslist;

	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex,
		Vec2i framesize
	);
};

#endif