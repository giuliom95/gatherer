#include "windowfilter.hpp"

WindowFilter::WindowFilter(Vec3f pos, Vec3f n, Vec2f s)
{
	filtertypename = "Window";
	
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

	position = pos;
	normal = n;
	size = s;
	updatematrices();
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

	const float s0 = size[0];
	const float s1 = size[1];
	const Vec3f v0 = transformPoint(o2w, {0, -.5f*s0, -.5f*s1});
	const Vec3f v1 = transformPoint(o2w, {0, 1.5f*s0, -.5f*s1});
	const Vec3f v2 = transformPoint(o2w, {0, -.5f*s0, 1.5f*s1});

	for(unsigned i = 0; i < npaths; ++i)
	{
		const unsigned path_i = gd.selectedpaths[i];
		const unsigned nbounces = gd.pathslength[path_i];
		const unsigned off = gd.firstbounceindexes[path_i];

		// Get first bounce
		Vec3f prev_b = fromVec3h(gd.bouncesposition[off]);

		//LOG(info) << prev_b;

		//Starts from the second one
		for(unsigned b_i = off+1; b_i < off + nbounces; b_i++)
		{
			const Vec3f  this_b = fromVec3h(gd.bouncesposition[b_i]);
			//LOG(info) << this_b;

			const Vec3f  rv = prev_b - this_b;
			const Vec3f& ro = this_b;
			constexpr float EPSILON = 0.0000001f;
			Vec3f edge1, edge2, h, s, q;
			float a,f,u,v;
			edge1 = v1 - v0;
			edge2 = v2 - v0;
			h = cross(rv, edge2);
			a = dot(edge1, h);
			if (a > -EPSILON && a < EPSILON)
			{
				prev_b = this_b;
				continue;
			}
			f = 1.0f/a;
			s = ro - v0;
			u = f * dot(s, h);
			// u < .5 because only one big triangle instead of two quads
			if (u < 0.0f || u > 0.5f)
			{
				prev_b = this_b;
				continue;
			}
			q = cross(s, edge1);
			v = f * dot(rv, q);
			if (v < 0.0f || v > 0.5f || u + v > 1.0f)
			{
				prev_b = this_b;
				continue;
			}

			float t = f * dot(edge2, q);

			if(t > 0 && t < 1)
			{
				selectedpaths.push_back(path_i);
				//LOG(info) << "FOUND";
				break;
			}
			
			prev_b = this_b;
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

	char label[13];

	sprintf(label, "Position##%u", globalid);
	modified |= ImGui::DragFloat3(
		label, reinterpret_cast<float*>(&position)
	);

	sprintf(label, "Normal##%u", globalid);
	if(ImGui::DragFloat3(
		label, reinterpret_cast<float*>(&normal), 0.01f
	)) {
		normal = normalize(normal);
		modified = true;
	}

	sprintf(label, "Size##%u", globalid);
	modified |= ImGui::DragFloat2(
		label, reinterpret_cast<float*>(&size)
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