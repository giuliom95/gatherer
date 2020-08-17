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
	void render(Camera& cam);

private:
	std::vector<Geometry> geometries;
	GLuint shaprog_idx;
	GLuint locid_camvpmat;
	GLuint locid_geomcolor;
	GLuint locid_eye;
};
#endif