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
	
	uintmax_t lenghts_bytesize   = boost::filesystem::file_size(lengths_fp);
	uintmax_t positions_bytesize = boost::filesystem::file_size(positions_fp);

	pathlengths.resize(lenghts_bytesize / sizeof(uint8_t));
	bouncepositions.resize(positions_bytesize / sizeof(Vec3h));

	lengths_ifs.read((char*)pathlengths.data(), lenghts_bytesize);
	positions_ifs.read((char*)bouncepositions.data(), positions_bytesize);

	lengths_ifs.close();
	positions_ifs.close();

	BOOST_LOG_TRIVIAL(info) << "Loaded all paths";
}