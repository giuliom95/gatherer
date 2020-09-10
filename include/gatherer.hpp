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
	boost::filesystem::ofstream	outstream;
	boost::filesystem::path		outfile;


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
	uint8_t 				currentpathbounces = 0;

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
		bouncespositions.outfile = folder / "bounces" / filename;
		bouncespositions.outstream.open(bouncespositions.outfile);

		std::sprintf(filename, "luminance%02d.bin", ti);
		luminance.outfile = folder / "paths" / filename;
		luminance.outstream.open(luminance.outfile);

		std::sprintf(filename, "lenghts%02d.bin", ti);
		lenghts.outfile = folder / "paths" / filename;
		lenghts.outstream.open(lenghts.outfile);

		std::sprintf(filename, "camerasamples%02d.bin", ti);
		camerasamples.outfile = folder / "paths" / filename;
		camerasamples.outstream.open(camerasamples.outfile);
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
		folderpath = folder;

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

		boost::filesystem::ofstream bpos_ofs{
			folderpath / "bounces" / "positions.bin"
		};
		boost::filesystem::ofstream lum_ofs{
			folderpath / "paths" / "luminance.bin"
		};
		boost::filesystem::ofstream len_ofs{
			folderpath / "paths" / "lenghts.bin"
		};
		boost::filesystem::ofstream cs_ofs{
			folderpath / "paths" / "camerasamples.bin"
		};

		for(DataBlock& db : data)
		{
			db.bouncespositions.storechunk();
			db.luminance.storechunk();
			db.lenghts.storechunk();
			db.camerasamples.storechunk();

			db.closefiles();

			std::vector<char> buf(1024);

			movefilecontents(db.bouncespositions.outfile, buf, bpos_ofs);
			movefilecontents(db.luminance.outfile, buf, lum_ofs);
			movefilecontents(db.lenghts.outfile, buf, len_ofs);
			movefilecontents(db.camerasamples.outfile, buf, cs_ofs);

			boost::filesystem::remove(db.bouncespositions.outfile);
			boost::filesystem::remove(db.luminance.outfile);
			boost::filesystem::remove(db.lenghts.outfile);
			boost::filesystem::remove(db.camerasamples.outfile);
		}

		bpos_ofs.close();
		lum_ofs.close();
		len_ofs.close();
		cs_ofs.close();
	}

	void addbounce(unsigned tid, Vec3h pos)
	{
		std::vector<Vec3h>& pd = data[tid].bouncespositions.data;
		data[tid].currentpathbounces++;
		pd.push_back(pos);
		if(pd.size() >= pd.capacity())
		{
			data[tid].bouncespositions.storechunk();
		}
	}

	void finalizepath(unsigned tid, Vec3h luminance, CameraSample sample)
	{
		DataBlock& db = data[tid];
		db.lenghts.data.push_back(db.currentpathbounces);
		db.currentpathbounces = 0;
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
	std::vector<DataBlock>	data;
	boost::filesystem::path	folderpath;

	void movefilecontents(
		boost::filesystem::path& ifs_path,
		std::vector<char>& buf,
		boost::filesystem::ofstream& ofs
	) {
		boost::filesystem::ifstream ifs{ifs_path};
		while(!ifs.eof())
		{
			ifs.read(buf.data(), buf.size());
			std::streamsize readbytes = ifs.gcount();
			ofs.write(buf.data(), readbytes);
		}
		ifs.close();
	}
};

#endif