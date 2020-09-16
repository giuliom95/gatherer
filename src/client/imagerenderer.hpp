#ifndef _IMAGERENDERER_HPP_
#define _IMAGERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "gathereddata.hpp"

class ImageRenderer
{
public:
	void init(GatheredData& gd);
	void render();
	void updatepathmask(GatheredData& gd);
	GLuint fbotex_id;
	Vec2i rendersize;
	float exposure;
private:
	std::vector<Vec3h> renderedimage;
	GLuint renderedimagetex_id;

	GLuint shaprog_id;
	GLuint fbo_id;
	GLuint pathmasktex_id;
	GLuint locid_renderedimagetex;
	GLuint locid_pathmasktex;
	GLuint locid_exposure;
};

#endif