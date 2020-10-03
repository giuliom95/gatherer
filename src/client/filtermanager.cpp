#include "filtermanager.hpp"

void FilterManager::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex,
	Vec2i framesize
) {
	for(Filter& f : filterslist)
	{
		f.render(cam, scenefbo_id, scenedepthtex, scenebeautytex, framesize);
	}
}