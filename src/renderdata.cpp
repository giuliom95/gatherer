#include "gatherer.hpp"

RenderData::RenderData(const unsigned nthreads) {
	pathgroups = std::vector<PathsGroup>(nthreads);
	for(unsigned pgi = 0; pgi < nthreads; ++pgi)
		pathgroups[pgi] = PathsGroup();
}

void RenderData::disk_load_all(const boost::filesystem::path& dirpath)
{
	/*
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
	*/
}

void RenderData::disk_store_all(const boost::filesystem::path& dirpath)
{
	if(!boost::filesystem::create_directory(dirpath))
	{
		BOOST_LOG_TRIVIAL(warning) << "Directory already there, overwriting";
	}
	boost::filesystem::ofstream lenghts_ofs{dirpath / "lenghts.bin"};
	boost::filesystem::ofstream paths_ofs{dirpath / "paths.bin"}; 
	
	unsigned long npaths = 0;

	for(const PathsGroup pg : pathgroups)
	{
		for(const Path& path : pg)
		{
			++npaths;
			const uint8_t npoints = (uint8_t)path.points.size();
			lenghts_ofs.write(reinterpret_cast<const char*>(&npoints), 1);
			paths_ofs.write
			(
				reinterpret_cast<const char*>(path.points.data()),
				npoints*sizeof(Vec3)
			);
		}
	}
	lenghts_ofs.close();
	paths_ofs.close();

	BOOST_LOG_TRIVIAL(info) << "Wrote " << npaths << " paths";
}