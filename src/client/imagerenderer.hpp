#ifndef _IMAGERENDERER_HPP_
#define _IMAGERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "gathereddata.hpp"

enum ImageDisplayMode : int
{
	fullrender,
	finalradiance,
	pathsperpixel
};

class ImageRenderer
{
public:
	void init(GatheredData& gd);
	void render();
	void updatepathmask(GatheredData& gd);
	GLuint fbotex_id;
	Vec2i rendersize;
	float exposure;
	Vec3f bgcolor;
	ImageDisplayMode displaymode;
private:
	std::vector<Vec3h> renderedimage;
	std::vector<uint16_t> pathmask;
	float spp_over_2e16;

	GLuint fullrendertex_id;

	GLuint shaprog_id;
	GLuint fbo_id;
	GLuint pathmasktex_id;
	GLuint locid_fullrendertex;
	GLuint locid_pathmasktex;
	GLuint locid_spp;
	GLuint locid_exposure;
	GLuint locid_bgcolor;
	GLuint locid_displaymode;
};

#endif