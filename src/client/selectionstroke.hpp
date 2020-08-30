#ifndef _SELECTIONSTROKE_HPP_
#define _SELECTIONSTROKE_HPP_

#include <set>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"
#include "gatherer.hpp"

#define SELECTIONSTROKE_DEFBRUSHSIZE 10

class Sphere
{
public:
	Vec3f center;
	float radius;
};

class SelectionStroke
{
public:
	void init();
	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex
	);

	std::set<unsigned> selectedpaths;

	void addpoint(Vec3f pt, RenderData& rd);

	float brushsize;

private:

	std::vector<Sphere> spheres;

	GLuint shaprog1_id;
	GLuint locid1_camvpmat;
	GLuint locid1_radius;
	GLuint locid1_location;
	GLuint locid1_scenedepth;

	GLuint shaprog2_id;
	GLuint locid2_scenebeauty;
	GLuint locid2_mask;

	GLuint fbo_id;
	GLuint texid_fbomask;
};

#endif