#ifndef _AXESVISUALIZER_HPP_
#define _AXESVISUALIZER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"

class AxesVisualizer
{
public:
	void init();
	void render(Camera& cam);
private:
	GLuint shaprog_id;
	GLuint fbo_id;
	GLuint fbotex_id;
	GLint  locid_camvpmat;
};

#endif