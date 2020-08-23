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

	GLuint texid_fboworldpos;
	GLuint fbo_id;
private:
	std::vector<Geometry> geometries;
	GLuint shaprog1_idx;
	GLuint locid1_camvpmat;
	GLuint locid1_geomcolor;
	GLuint locid1_eye;

	GLuint shaprog2_idx;
	GLuint locid2_albedotex;

	GLuint texid_fboalbedo;
};
#endif