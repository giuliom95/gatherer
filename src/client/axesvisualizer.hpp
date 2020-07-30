#ifndef _AXESVISUALIZER_HPP_
#define _AXESVISUALIZER_HPP_

#define AXESVISUZLIZER_W 64
#define AXESVISUZLIZER_H 64

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
	GLuint fbotex_id;
private:
	GLuint shaprog_id;
	GLuint fbo_id;
	GLint  locid_camvpmat;
};

#endif