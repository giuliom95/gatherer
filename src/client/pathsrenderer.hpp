#ifndef _PATHSRENDERER_HPP_
#define _PATHSRENDERER_HPP_

#include <set>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "camera.hpp"

#include "gathereddata.hpp"

#define PATHSRENDERER_DEFPATHSALPHA 0.2f

class PathsRenderer
{
public:
	void init();
	void render(
		Camera& cam,
		GLuint fbo,
		GLuint scenedepthtex,
		Vec2i framesize,
		GatheredData& gd
	);
	
	bool enablerendering;

	float pathsalpha;
	bool enabledepth;
	bool enableradiance;

	void updaterenderlist(GatheredData& rd);
	
private:
	GLuint vaoidx;
	GLuint posvboidx;
	GLuint colorvboidx;
	GLuint shaprog_idx;
	GLuint locid_camvpmat;
	GLuint locid_pathsalpha;
	GLuint locid_enabledepth;
	GLuint locid_enableradiance;
	GLuint locid_scenedepth;
	GLuint locid_framesize;

	std::vector<GLint>		paths_firsts;
	std::vector<GLsizei>	paths_lengths;
};

#endif