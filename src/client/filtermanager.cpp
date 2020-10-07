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
	unsigned i = 0;
	for(
		std::list<std::shared_ptr<Filter>>::iterator it = filterslist.begin();
		it != filterslist.end();
	) {
		char label[6];
		sprintf(label, "-##%02u", i);
		
		bool toerase = ImGui::Button(label);
		ImGui::SameLine();
		ImGui::Text("Filter %u", (*it)->globalid);
		modified |= toerase;
		if(toerase)
		{
			it = filterslist.erase(it);
		}
		if(!toerase) ++it;
		++i;
	}
	return modified;
}

void FilterManager::computepaths(GatheredData& gd)
{
	gd.selectedpaths.resize(gd.npaths);
	std::iota(gd.selectedpaths.begin(), gd.selectedpaths.end(), 0);

	for(std::shared_ptr<Filter> f : filterslist)
	{
		f->computepaths(gd);
	}
}