#ifndef _GATHERER_HPP_
#define _GATHERER_HPP_

#include "math.hpp"

#include <vector>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

class Path
{
public:
	Path() : Path(0) {}
	Path(const size_t npoints)
	{
		points = std::vector<Vec3h>(npoints);
	}

	void add_point(const Vec3h& p)
	{
		points.push_back(p);
	}
	
	std::vector<Vec3h> points;
};

using PathsGroup = std::vector<Path>;


class RenderData
{
public:
	RenderData(const unsigned nthreads=1)
	{
		pathgroups = std::vector<PathsGroup>(nthreads);
		for(unsigned pgi = 0; pgi < nthreads; ++pgi)
			pathgroups[pgi] = PathsGroup();
	}

	void disk_load_all (
		const boost::filesystem::path& dirpath
	) {
		BOOST_LOG_TRIVIAL(info) << "+ Loading paths";
		const boost::filesystem::path lengths_fp = dirpath / "lengths.bin";
		const boost::filesystem::path paths_fp   = dirpath / "paths.bin";
		boost::filesystem::ifstream lengths_ifs(lengths_fp);
		boost::filesystem::ifstream paths_ifs  (paths_fp);

		pathgroups = std::vector<PathsGroup>(1);
		pathgroups[0] = PathsGroup();

		while(!lengths_ifs.eof())
		{
			// Number of points in path 
			uint8_t npoints;
			lengths_ifs.read((char*)&npoints, sizeof(uint8_t));

			Path path(npoints);
			paths_ifs.read
			(
				(char*)path.points.data(), 
				npoints*sizeof(Vec3h)
			);

			pathgroups[0].push_back(path);
		}
		lengths_ifs.close();
		paths_ifs.close();

		BOOST_LOG_TRIVIAL(info) << "Loaded " << pathgroups[0].size() << " paths";
	}

	
	void disk_store_all(
		const boost::filesystem::path& dirpath
	) {
		if(!boost::filesystem::create_directory(dirpath))
		{
			BOOST_LOG_TRIVIAL(warning) << "Directory already there, overwriting";
		}
		boost::filesystem::ofstream lenghts_ofs{dirpath / "lengths.bin"};
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
					npoints*sizeof(Vec3h)
				);
			}
		}
		lenghts_ofs.close();
		paths_ofs.close();

		BOOST_LOG_TRIVIAL(info) << "Wrote " << npaths << " paths";
	}

	std::vector<PathsGroup> pathgroups;
};

#endif