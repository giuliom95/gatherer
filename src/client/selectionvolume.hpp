#ifndef _SELECTIONVOLUME_HPP_
#define _SELECTIONVOLUME_HPP_

#include <set>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"
#include "gatherer.hpp"

class SelectionVolume {
public:
	void init();
	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex
	);

	Vec3f location;
	float radius;
	std::set<unsigned> selectedpaths;

	void selectpaths(RenderData& rd);

private:

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