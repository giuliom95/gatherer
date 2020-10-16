#ifndef _SCENERENDERER_HPP_
#define _SCENERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "camera.hpp"

#include "json.hpp"

class Geometry
{
public:
	unsigned offset;
	unsigned count;
	Vec3f albedo;
};

class SceneRenderer
{
public:
	void init(Camera& cam);
	void render1(Camera& cam);
	void render2();

	void setframesize(Vec2i size);

	//AABB bbox;

	GLuint texid_fboworldpos;
	GLuint texid_fbobeauty;
	GLuint texid_fbodepth;
	GLuint fbo_id;
private:
	GLuint vaoidx;

	unsigned nverts;
	unsigned nidxs;

	std::vector<Geometry> geometries;
	GLuint shaprog1_idx;
	GLuint locid1_camvpmat;
	GLuint locid1_geomalbedo;
	GLuint locid1_eye;

	GLuint shaprog2_idx;
	GLuint locid2_beautytex;

};
#endif