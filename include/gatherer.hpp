#ifndef _GATHERER_HPP_
#define _GATHERER_HPP_

#include "math.hpp"
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

template <class Vec3>
class Path
{
public:
	Path() : Path(0) {}
	Path(const size_t npoints)
	{
		points = std::vector<Vec3>(npoints);
	}

	void add_point(const Vec3& p)
	{
		points.push_back(p);
	}
	
	std::vector<Vec3> points;
};

template <class Vec3>
using PathsGroup = std::vector<Path<Vec3>>;

template <class Vec3>
class RenderData
{
public:
	RenderData(const unsigned nthreads=1)
	{
		pathgroups = std::vector<PathsGroup<Vec3>>(nthreads);
		for(unsigned pgi = 0; pgi < nthreads; ++pgi)
			pathgroups[pgi] = PathsGroup<Vec3>();
	}

	/*
	void disk_load_all (
		const boost::filesystem::path& dirpath
	) {
		boost::filesystem::ifstream lenghts_ofs{dirpath / "lengths.bin"};
		boost::filesystem::ifstream paths_ofs{dirpath / "paths.bin"}; 

		while(lenghts_ofs.eof())
		{
			// Number of points in path 
			uint8_t npoints;
			lenghts_ofs.read((char*)&npoints, sizeof(uint8_t));

			Path path(npoints);
			paths_ofs.read
			(
				(char*)path.points.data(), 
				npoints*sizeof(Vec3h)
			);

			paths.push_back(path);
		}

		filestream.close();

		BOOST_LOG_TRIVIAL(info) << "Loaded " << paths.size() << " paths";
	}
	*/
	
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

		for(const PathsGroup<Vec3> pg : pathgroups)
		{
			for(const Path<Vec3>& path : pg)
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

	std::vector<PathsGroup<Vec3>> pathgroups;
};

#endif