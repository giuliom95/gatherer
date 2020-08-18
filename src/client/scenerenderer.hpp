#ifndef _SCENERENDERER_HPP_
#define _SCENERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "camera.hpp"

class Geometry
{
public:
	GLuint vaoidx;
	unsigned nelems;
	Vec3f color;
};

class SceneRenderer
{
public:
	void init();
	void render1(Camera& cam);
	void render2();

private:
	std::vector<Geometry> geometries;
	GLuint shaprog1_idx;
	GLuint locid1_camvpmat;
	GLuint locid1_geomcolor;

	GLuint fbo_idx;
	GLuint fbocolortex_idx;
	GLuint fbodepthtex_idx;
};
#endif