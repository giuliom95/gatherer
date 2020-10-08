#ifndef _SELECTIONSTROKE_HPP_
#define _SELECTIONSTROKE_HPP_

#include "filtermanager.hpp"

#include <set>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "utils.hpp"
#include "camera.hpp"
#include "gathereddata.hpp"

#define SELECTIONSPHERE_DEFRADIUS 30

class SelectionSphere : public Filter
{
public:
	SelectionSphere(Vec3f c, float r = 0);

	void render(
		Camera& cam, 
		GLuint scenefbo_id, 
		GLuint scenedepthtex,
		GLuint scenebeautytex
	) override;

	void setframesize(Vec2i size) override;

	void computepaths(GatheredData& gd) override;

	bool renderstackui() override;

	Vec3f center;
	float radius;

private:

	GLuint shaprog1_id;
	GLuint locid1_camvpmat;
	GLuint locid1_radius;
	GLuint locid1_location;
	GLuint locid1_scenedepth;
	GLuint locid1_framesize;

	GLuint shaprog2_id;
	GLuint locid2_scenebeauty;
	GLuint locid2_mask;

	GLuint fbo_id;
	GLuint texid_fbomask;
};

#endif