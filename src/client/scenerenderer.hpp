#ifndef _SCENERENDERER_HPP_
#define _SCENERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "camera.hpp"

class SceneRenderer
{
public:
	void init();
	void render(Camera& cam);

private:
	GLuint vaoidx;
	GLuint vboidx;
	GLuint eboidx;
	GLuint shaprog_idx;
	GLuint locid_camvpmat;
};

#endif