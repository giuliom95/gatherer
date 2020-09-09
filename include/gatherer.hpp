#ifndef _GATHERER_HPP_
#define _GATHERER_HPP_

#include "math.hpp"

#include <vector>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

class CameraSample
{
public:
	uint16_t i, j;
	half u, v;
};

template<typename T>
class Buffer
{
public:
	std::vector<T> data;
	boost::filesystem::ofstream outstream;

	void storechunk()
	{
		outstream.write(
			reinterpret_cast<char*>(data.data()),
			data.size() * sizeof(T)
		);
		data.clear();
	}
};

class DataBlock
{
public:
	static constexpr size_t maxpaths = 128*64*64;
	static constexpr size_t maxbounces = 10;
	static constexpr size_t maxgeom = maxpaths*maxbounces;

	Buffer<Vec3h>			bouncespositions;

	Buffer<Vec3h>			luminance;
	Buffer<uint8_t>			lenghts;
	Buffer<CameraSample>	camerasamples;

	void reserve()
	{
		bouncespositions.data.reserve(maxgeom);
		luminance.data.reserve(maxpaths);
		lenghts.data.reserve(maxpaths);
		camerasamples.data.reserve(maxpaths);
	}

	void openfiles(unsigned ti, const boost::filesystem::path& folder)
	{
		char filename[20];
		std::sprintf(filename, "positions%02d.bin", ti);
		const boost::filesystem::path bppath = folder / "bounces" / filename;
		BOOST_LOG_TRIVIAL(info) << bppath;
		bouncespositions.outstream.open(bppath);

		std::sprintf(filename, "luminance%02d.bin", ti);
		const boost::filesystem::path lumpath = folder / "paths" / filename;
		BOOST_LOG_TRIVIAL(info) << lumpath;
		luminance.outstream.open(lumpath);

		std::sprintf(filename, "lenghts%02d.bin", ti);
		const boost::filesystem::path lenpath = folder / "paths" / filename;
		BOOST_LOG_TRIVIAL(info) << lenpath;
		lenghts.outstream.open(lenpath);

		std::sprintf(filename, "camerasamples%02d.bin", ti);
		const boost::filesystem::path cspath = folder / "paths" / filename;
		BOOST_LOG_TRIVIAL(info) << cspath;
		camerasamples.outstream.open(cspath);
	}

	void closefiles()
	{
		bouncespositions.outstream.close();
		luminance.outstream.close();
		lenghts.outstream.close();
		camerasamples.outstream.close();
	}
};


class Gatherer
{
public:
	Gatherer(unsigned nthreads, const boost::filesystem::path& folder)
	{
		BOOST_LOG_TRIVIAL(info) << "Initializing Gatherer";
		data = std::vector<DataBlock>(nthreads);

		for(unsigned ti = 0; ti < nthreads; ++ti)
		{
			DataBlock& db = data[ti];
			db.reserve();
			db.openfiles(ti, folder);
		}
	}

	~Gatherer()
	{
		BOOST_LOG_TRIVIAL(info) << "Releasing Gatherer";
		for(DataBlock& db : data)
		{
			db.bouncespositions.storechunk();
			db.lenghts.storechunk();
			db.luminance.storechunk();
			db.camerasamples.storechunk();
			db.closefiles();
		}
	}

	void addbounce(unsigned tid, Vec3h pos)
	{
		std::vector<Vec3h>& pd = data[tid].bouncespositions.data;
		pd.push_back(pos);
		if(pd.size() >= pd.capacity())
		{
			data[tid].bouncespositions.storechunk();
		}
	}

	void finalizepath(unsigned tid, Vec3h luminance, CameraSample sample)
	{
		DataBlock& db = data[tid];
		db.lenghts.data.push_back(0);
		db.luminance.data.push_back(luminance);
		db.camerasamples.data.push_back(sample);

		std::vector<uint8_t>& ld = db.lenghts.data;
		if(ld.size() >= ld.capacity())
		{
			data[tid].lenghts.storechunk();
			data[tid].luminance.storechunk();
			data[tid].camerasamples.storechunk();
		}
	}

private:
	std::vector<DataBlock> data;
};
/*
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
*/
#endif