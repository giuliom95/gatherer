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

void FilterManager::addfilter(std::shared_ptr<Filter> filter)
{
	filterslist.push_back(filter);
	filter->globalid = progressiveid;
	progressiveid++;
}


void FilterManager::setframesize(Vec2i size)
{
	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->setframesize(size);
	}
}

bool FilterManager::renderui()
{
	bool modified = false;
	for(
		std::list<std::shared_ptr<Filter>>::iterator it = filterslist.begin();
		it != filterslist.end();
	) {
		ImGui::Text("Filter %u", (*it)->globalid);
		bool toerase = ImGui::IsItemClicked();
		modified |= toerase;
		if(toerase)
		{
			it = filterslist.erase(it);
		}
		if(!toerase) ++it;
	}
	return modified;
}

void FilterManager::computepaths(GatheredData& gd)
{
	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->computepaths(gd);
	}
}