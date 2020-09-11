#ifndef _GATHEREDDATA_HPP_
#define _GATHEREDDATA_HPP_

#include "utils.hpp"

class GatheredData
{
public:
	boost::filesystem::path datafolder;

	std::vector<uint8_t> pathslength;
	std::vector<Vec3h> bouncesposition;

	std::vector<unsigned> firstbounceindexes;

	void loadall(const boost::filesystem::path& folder);
};

#endif