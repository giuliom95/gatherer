#include "gatherer.hpp"

RenderData::RenderData(const boost::filesystem::path& in_filepath)
{
	filepath = in_filepath;
	pathgroups = std::vector<PathsGroup>();
}

void RenderData::disk_load_all()
{
	std::ifstream filestream{filepath.string()};

	pathgroups = std::vector<PathsGroup>(1);

	while(filestream.eof())
	{
		// Number of points in path 
		uint8_t npoints;
		filestream.read
		(
			reinterpret_cast<char*>(&npoints), 
			sizeof(uint8_t)
		);

		Path path(npoints);
		filestream.read
		(
			reinterpret_cast<char*>(path.points.data()), 
			npoints*sizeof(Vec3h)
		);
		
		pathgroups[0].push_back(path);
	}

	filestream.close();

	BOOST_LOG_TRIVIAL(info) << "Loaded";
}

void RenderData::disk_store_all()
{
	std::ofstream filestream{filepath.string()};

	for(const PathsGroup pg : pathgroups)
	{
		for(const Path& path : pg)
		{
			uint8_t npoints = (uint8_t)path.points.size();
			filestream.put(npoints);
			filestream.write
			(
				reinterpret_cast<const char*>(path.points.data()),
				npoints*sizeof(Vec3h)
			);
		}
	}
	filestream.close();

	BOOST_LOG_TRIVIAL(info) << "Wrote paths";
}