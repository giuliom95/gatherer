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
	void render(Camera& cam, GLuint scenedepthtex, Vec2i framesize);
	
	float pathsalpha;
	bool enabledepth;
	
	std::set<unsigned> selectedpaths;

	void addpaths(std::set<unsigned>& paths);
	void removepaths(std::set<unsigned>& paths);

	void clearpaths();

	void updaterenderlist(GatheredData& rd);
	
private:
	GLuint vaoidx;
	GLuint vboidx;
	GLuint shaprog_idx;
	GLuint locid_camvpmat;
	GLuint locid_pathsalpha;
	GLuint locid_enabledepth;
	GLuint locid_scenedepth;
	GLuint locid_framesize;


	std::vector<GLint>		paths_firsts;
	std::vector<GLsizei>	paths_lengths;
};

#endif