#ifndef _PATHSRENDERER_HPP_
#define _PATHSRENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "camera.hpp"

#include "gatherer.hpp"

#define PATHSRENDERER_DEFPATHSALPHA 0.2f

class PathsRenderer
{
public:
	void init();
	void render(Camera& cam, GLuint scenedepthtex);
	
	float pathsalpha;
	bool enabledepth;

	void pathsbouncinginsphere(Vec3f center, float radius);
private:
	void disk_load_all_paths(
		const boost::filesystem::path dirpath
	);

	GLuint vaoidx;
	GLuint vboidx;
	GLuint shaprog_idx;
	GLuint locid_camvpmat;
	GLuint locid_pathsalpha;
	GLuint locid_enabledepth;
	GLuint locid_scenedepth;

	RenderData renderdata;
	unsigned				paths_number;
	std::vector<GLint>		paths_firsts;
	std::vector<GLsizei>	paths_lenghts;
};

#endif