#include "gatherer.hpp"

Path::Path() : Path(0) {}

Path::Path(const size_t npoints)
{
	points = std::vector<Vec3>(npoints);
}

void Path::add_point(const Vec3& p)
{
	//points.push_back(fromVec3f(p));
	points.push_back(p);
}