#ifndef _GATHERER_HPP_
#define _GATHERER_HPP_

#include "math.hpp"
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

class Path
{
public:
	Path();
	Path(const size_t npoints);
	void add_point(const Vec3f& p);
	std::vector<Vec3h> points;
};

using PathsGroup = std::vector<Path>;

class RenderData
{
public:
	RenderData(const boost::filesystem::path& in_filepath);
	void disk_load_all();
	void disk_store_all();

	boost::filesystem::path	filepath;
	std::vector<PathsGroup>	pathgroups;
};

#endif