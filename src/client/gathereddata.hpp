#ifndef _GATHEREDDATA_HPP_
#define _GATHEREDDATA_HPP_

#include "utils.hpp"

#include "json.hpp"
#include "gatherer.hpp"

class GatheredData
{
public:
	boost::filesystem::path datafolder;

	Vec2i rendersize;
	unsigned rendersamples;

	unsigned npaths;
	unsigned nbounces;
	std::vector<uint8_t> pathslength;
	std::vector<Vec3h> bouncesposition;
	std::vector<Vec3h> pathsradiance;
	std::vector<CameraSample> pathscamerasamples;

	std::vector<unsigned> firstbounceindexes;

	std::vector<unsigned> selectedpaths;
	// Used to store temporary data;
	std::vector<unsigned> selectedpathstmpbuf;

	void loadall(
		const boost::filesystem::path& folder,
		const boost::filesystem::path& scenejson
	);
};

#endif