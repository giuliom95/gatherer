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

	while(!filestream.eof())
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
			npoints*sizeof(Vec3)
		);
		
		pathgroups[0].push_back(path);
	}

	filestream.close();

	BOOST_LOG_TRIVIAL(info) << "Loaded";
}

void RenderData::disk_store_all()
{
	std::ofstream filestream{filepath.string()};
	unsigned long npaths = 0;

	for(const PathsGroup pg : pathgroups)
	{
		for(const Path& path : pg)
		{
			++npaths;
			const uint8_t npoints = (uint8_t)path.points.size();
			BOOST_LOG_TRIVIAL(info) << "Path npoints=" << (int)npoints;
			filestream.write(reinterpret_cast<const char*>(&npoints), 1);
			filestream.write
			(
				reinterpret_cast<const char*>(path.points.data()),
				npoints*sizeof(Vec3)
			);
		}
	}
	filestream.close();

	BOOST_LOG_TRIVIAL(info) << "Wrote " << npaths << " paths";
}