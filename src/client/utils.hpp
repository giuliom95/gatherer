#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "math.hpp"

#define DEF_WINDOW_W 1024
#define DEF_WINDOW_H 1024

#define LOG BOOST_LOG_TRIVIAL

class AABB {
public:
	AABB();
	AABB(std::vector<Vec3f>& points);
	AABB(Vec3f min, Vec3f max);

	Vec3f minpt, maxpt;

	void addpt(Vec3f pt);
	Vec3f center();
	float maxlength();
};

GLuint disk_load_shader(
	const boost::filesystem::path&	path,
	const GLenum 					type
);

GLuint disk_load_shader_program(
	const boost::filesystem::path& vtxsha_path,
	const boost::filesystem::path& fragsha_path,
	const boost::filesystem::path& tessha_path = "",
	const boost::filesystem::path& geomsha_path = ""
);

bool glfwCheckErrors();

Vec2f get_cursor_pos(GLFWwindow* window);

// sph[0] = r, sph[1] = yaw, sph[2] = pitch
Vec3f spherical2cartesian(Vec3f sph);
Vec3f cartesian2spherical(Vec3f vec);


#endif