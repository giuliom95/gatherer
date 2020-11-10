#ifndef _SCENERENDERER_HPP_
#define _SCENERENDERER_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/convenience.hpp>

#include "utils.hpp"
#include "camera.hpp"

#include "json.hpp"

class Geometry
{
public:
	unsigned offset;
	unsigned count;
	Vec3f color;
	float alpha;
	bool visible = true;
	bool backfaceculling = true;
	std::string name;
};

class Triangle
{
public:
	Vec2f o;
	Vec2f v1;
	Vec2f v2;
};


class SceneRenderer
{
public:
	void init(const boost::filesystem::path& path, Camera& cam);
	bool renderui();
	void render1(Camera& cam, bool opaque = true);
	void render2(GLuint final_fbo);
	void render3(GLuint final_fbo, GLuint finaltex);

	void setframesize(Vec2i size);

	AABB bbox;

	GLuint texid_fboworldposid;

	GLuint texid_opaquebeauty;
	GLuint texid_opaquedepth;
	GLuint opaquefbo_id;

	GLuint texid_transparentbeauty;
	GLuint texid_transparentdepth;
	GLuint transparentfbo_id;

	Vec3f blend_color;
	float blend_alpha;

	int selected_geom = -1;
private:
	GLuint vaoidx;

	unsigned nverts;
	unsigned nidxs;

	std::vector<Geometry> geometries;
	GLuint shaprog1_idx;
	GLuint locid1_camvpmat;
	GLuint locid1_geocolor;
	GLuint locid1_geomalpha;
	GLuint locid1_eye;
	GLuint locid1_blend;
	GLuint locid1_geomid;
	GLuint locid1_opaquedepth;
	GLuint locid1_highlight;

	GLuint shaprog2_idx;
	GLuint locid2_opaquebeauty;

	GLuint shaprog3_idx;
	GLuint locid3_finaltex;
	GLuint locid3_transparentbeauty;

	void loadscene(const boost::filesystem::path& path, Camera& cam);
	void generateopaquefbo();
	void generatetransparentfbo();

	void generateuvmap(
		const std::vector<Vec3f>& vertices, 
		const std::vector<unsigned>& indexes
	);

	bool visibilitytoggle = false;
	bool backfacestoggle = false;
};
#endif