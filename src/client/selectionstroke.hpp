#ifndef _SELECTIONSTROKE_HPP_
#define _SELECTIONSTROKE_HPP_

#include <set>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"
#include "gathereddata.hpp"

#define SELECTIONSTROKE_DEFBRUSHSIZE 30

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
		GLuint scenebeautytex,
		Vec2i framesize
	);

	void setframesize(Vec2i size);

	std::set<unsigned> selectedpaths;

	void addpoint(Vec3f pt);

	void findbounces(GatheredData& gd);

	void clearpoints();

	float brushsize;

private:

	std::vector<Sphere> spheres;

	GLuint shaprog1_id;
	GLuint locid1_camvpmat;
	GLuint locid1_radius;
	GLuint locid1_location;
	GLuint locid1_scenedepth;
	GLuint locid1_framesize;

	GLuint shaprog2_id;
	GLuint locid2_scenebeauty;
	GLuint locid2_mask;

	GLuint fbo_id;
	GLuint texid_fbomask;
};

#endif