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
	void render(Camera& cam);
private:
};

#endif