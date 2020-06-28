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
	std::vector<Vec3h> points;
};

class RenderData
{
public:
	RenderData(const boost::filesystem::path& in_filepath);
	void disk_load_all();
	void disk_store_all();

private:
	boost::filesystem::path	filepath;
	std::vector<Path>		paths;
};

#endif