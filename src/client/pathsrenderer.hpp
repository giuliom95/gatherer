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
		GLuint finalfbo,
		GLuint opaquebeautytex,
		GLuint transbeautytex,
		GLuint opaquedepthtex,
		GLuint transdepthtex,
		GatheredData& gd
	);
	void setframesize(Vec2i size);
	
	bool enablerendering;

	float pathsalpha;
	bool enabledepth;
	bool enableradiance;

	void updaterenderlist(GatheredData& rd);
	
private:
	GLuint vaoidx;
	GLuint posvboidx;
	GLuint ssboidx;

	GLuint shaprog1_idx;
	GLuint locid1_camvpmat;
	GLuint locid1_pathsalpha;
	GLuint locid1_enabledepth;
	GLuint locid1_enableradiance;
	GLuint locid1_opaquedepth;
	GLuint locid1_transdepth;

	GLuint shaprog2_idx;
	GLuint locid2_opaquebeauty;
	GLuint locid2_transbeauty;
	GLuint locid2_above;
	GLuint locid2_below;

	GLuint texid_above;
	GLuint texid_below;
	GLuint fbo_id;

	std::vector<GLint>		paths_firsts;
	std::vector<GLsizei>	paths_lengths;
};

#endif