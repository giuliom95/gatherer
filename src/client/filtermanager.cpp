#include "filtermanager.hpp"

void FilterManager::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex,
	Vec2i framesize
) {
	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->render(cam, scenefbo_id, scenedepthtex, scenebeautytex, framesize);
	}
}

void FilterManager::setframesize(Vec2i size)
{
	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->setframesize(size);
	}
}

void FilterManager::renderui()
{
	for(std::shared_ptr<Filter> f : filterslist)
	{
		ImGui::Button("-"); ImGui::SameLine();
		ImGui::Text("filter 1");
	}
}

void FilterManager::computepaths(GatheredData& gd)
{
	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->computepaths(gd);
	}
}