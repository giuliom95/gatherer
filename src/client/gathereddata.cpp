#include "gathereddata.hpp"

void GatheredData::loadall(const boost::filesystem::path& folder)
{
	datafolder = folder;

	const boost::filesystem::path lengths_fp = 
		folder / "paths" / "lengths.bin";
	const boost::filesystem::path positions_fp = 
		folder / "bounces" / "positions.bin";

	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream positions_ifs(positions_fp);
	
	const unsigned lenghts_bytesize   = boost::filesystem::file_size(lengths_fp);
	const unsigned positions_bytesize = boost::filesystem::file_size(positions_fp);

	const unsigned npaths = lenghts_bytesize / sizeof(uint8_t);
	const unsigned nbounces = positions_bytesize / sizeof(Vec3h);

	pathslength.resize(npaths);
	bouncesposition.resize(nbounces);

	lengths_ifs.read((char*)pathslength.data(), lenghts_bytesize);
	positions_ifs.read((char*)bouncesposition.data(), positions_bytesize);

	lengths_ifs.close();
	positions_ifs.close();

	firstbounceindexes.reserve(npaths);
	unsigned off = 0;
	for(uint8_t l : pathslength)
	{
		firstbounceindexes.push_back(off);
		off += l;
	}

	BOOST_LOG_TRIVIAL(info) << "Loaded all paths";
}