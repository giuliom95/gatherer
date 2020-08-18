#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "math.hpp"

#define WINDOW_W 1024
#define WINDOW_H 1024

class AABB {
public:
	Vec3f min, max;
	Vec3f center() {
		return 0.5f * (min + max);
	}
};

GLuint disk_load_shader(
	const boost::filesystem::path&	path,
	const GLenum 					type
);

GLuint disk_load_shader_program(
	const boost::filesystem::path& vtxsha_path,
	const boost::filesystem::path& fragsha_path,
	const boost::filesystem::path& geomsha_path = ""
);

#endif