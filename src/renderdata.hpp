#ifndef _RENDERDATA_HPP_
#define _RENDERDATA_HPP_

#include <vector>
#include "path.hpp"
#include <boost/filesystem.hpp>


class RenderData
{
public:
	RenderData(const boost::filesystem::path& filepath);
	void disk_load_all();
	void disk_store_all();

private:
	boost::filesystem::path	file;
	std::vector<Path>		paths;
};

#endif