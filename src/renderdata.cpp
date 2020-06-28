#include "renderdata.hpp"

RenderData::RenderData(const boost::filesystem::path& in_filepath)
{
	filepath = in_filepath;
	paths = std::vector<Path>();
}

void RenderData::disk_load_all()
{
	std::ifstream filestream{filepath.string()};

	while(filestream.eof())
	{
		// Number of points in path 
		uint8_t npoints;
		filestream.read((char*)&npoints, sizeof(uint8_t));

		Path path(npoints);
		filestream.read
		(
			(char*)path.points.data(), 
			npoints*sizeof(Vec3h)
		);
		
		paths.push_back(path);
	}

	filestream.close();

	BOOST_LOG_TRIVIAL(info) << "Loaded " << paths.size() << " paths";
}

void RenderData::disk_store_all()
{
	
}