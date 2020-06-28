#include "gatherer.hpp"

Path::Path() : Path(0) {}

Path::Path(const size_t npoints)
{
	points = std::vector<Vec3h>(npoints);
}