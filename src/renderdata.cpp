#include "renderdata.hpp"

RenderData::RenderData(const boost::filesystem::path& filepath)
{
	file  = filepath;
	paths = std::vector<Path>();
}

void RenderData::disk_load_all()
{

}

void RenderData::disk_store_all()
{
	
}