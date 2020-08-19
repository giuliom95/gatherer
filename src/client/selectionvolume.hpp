#ifndef _SELECTIONVOLUME_HPP_
#define _SELECTIONVOLUME_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"

class SelectionVolume {
public:
	void init();
	void render(Camera& cam, GLuint texid_scenedepth);
private:
	Vec3f location;
	float radius;

	GLuint shaprog_id;
	GLuint locid_camvpmat;
	GLuint locid_plane2w;
	GLuint locid_radius;
	GLuint locid_scenedepth;
};

#endif