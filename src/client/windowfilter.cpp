#include "windowfilter.hpp"

WindowFilter::WindowFilter()
{
	shaprog_id = disk_load_shader_program(
		"../src/client/shaders/windowfilter.vert.glsl",
		"../src/client/shaders/windowfilter.frag.glsl"
	);

	locid_camvpmat    = glGetUniformLocation(shaprog_id, "vpmat");
	locid_scenedepth  = glGetUniformLocation(shaprog_id, "scenedepth");
	locid_framesize   = glGetUniformLocation(shaprog_id, "framesize");
	locid_scenebeauty = glGetUniformLocation(shaprog_id, "scenebeauty");

	locid_objmat      = glGetUniformLocation(shaprog_id, "objmat");
	locid_size        = glGetUniformLocation(shaprog_id, "size");
}

void WindowFilter::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex
) {
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	glUseProgram(shaprog_id);
	glBindFramebuffer(GL_FRAMEBUFFER, scenefbo_id);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform2i(locid_framesize, framesize[0], framesize[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid_scenedepth, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scenebeautytex);
	glUniform1i(locid_scenebeauty, 1);

	glUniformMatrix4fv(
		locid_objmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&o2w)
	);
	glUniform2f(locid_size, size[0], size[1]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void WindowFilter::computepaths(GatheredData& gd)
{
	// Do nothing for now
	const unsigned npaths = gd.selectedpaths.size();

	std::vector<unsigned> selectedpaths;
	selectedpaths.reserve(npaths);

	for(unsigned i = 0; i < npaths; ++i)
	{
		const unsigned path_i = gd.selectedpaths[i];
		const unsigned nbounces = gd.pathslength[path_i];
		const unsigned off = gd.firstbounceindexes[path_i];

		// Get first bounce
		Vec3f prev_b = fromVec3h(gd.bouncesposition[off]);
		// Transform it
		Vec3f t_prev_b = transformPoint(w2o, prev_b);

		LOG(info) << t_prev_b;

		//Starts from the second one
		for(unsigned b_i = off+1; b_i < off + nbounces; b_i++)
		{
			Vec3f this_b = fromVec3h(gd.bouncesposition[b_i]);
			Vec3f t_this_b = transformPoint(w2o, this_b);

			LOG(info) << t_this_b;

			// On the opposite half spaces
			if(t_this_b[0] * t_prev_b[0] < 1)
			{
				selectedpaths.push_back(path_i);
				LOG(info) << "FOUND";
				break;
			}

			prev_b = this_b;
			t_prev_b = t_this_b;
		}
	}

	unsigned newnpaths = selectedpaths.size();

	std::move(
		selectedpaths.begin(), 
		selectedpaths.end(), 
		gd.selectedpaths.begin()
	);

	gd.selectedpaths.erase(
		gd.selectedpaths.begin() + newnpaths, 
		gd.selectedpaths.end()
	);

	LOG(info) << "Filtered paths (" << newnpaths << " / " << npaths << ")";
}

bool WindowFilter::renderstackui()
{
	bool modified = false;

	modified |= ImGui::DragFloat3(
		"Position", reinterpret_cast<float*>(&position)
	);
	if(ImGui::DragFloat3(
		"Normal", reinterpret_cast<float*>(&normal), 0.01f
	)) {
		normal = normalize(normal);
		modified = true;
	}
	modified |= ImGui::DragFloat2(
		"Size", reinterpret_cast<float*>(&size)
	);

	if(modified) updatematrices();
	
	return modified;
}

void WindowFilter::updatematrices()
{
	const Mat4f axes(refFromVec(normal));
	const Mat4f axes_t(transpose(axes));
	o2w = axes * translationMatrix(position);
	w2o = translationMatrix(-1*position) * axes_t;
}