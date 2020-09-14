#ifndef _GATHEREDDATA_HPP_
#define _GATHEREDDATA_HPP_

#include "utils.hpp"

#include "json.hpp"

class GatheredData
{
public:
	boost::filesystem::path datafolder;

	Vec2i rendersize;
	unsigned rendersamples;

	std::vector<uint8_t> pathslength;
	std::vector<Vec3h> bouncesposition;
	std::vector<Vec3h> pathsluminance;

	std::vector<unsigned> firstbounceindexes;

	void loadall(const boost::filesystem::path& folder);
};

#endif