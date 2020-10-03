#include "selectionsphere.hpp"

SelectionSphere::SelectionSphere(Vec3f c, float r)
{
	shaprog1_id = disk_load_shader_program(
		"../src/client/shaders/selectionvolume1.vert.glsl",
		"../src/client/shaders/selectionvolume1.frag.glsl",
		"../src/client/shaders/selectionvolume1.tes.glsl"
	);

	locid1_camvpmat   = glGetUniformLocation(shaprog1_id, "vpmat");
	locid1_radius     = glGetUniformLocation(shaprog1_id, "radius");
	locid1_location   = glGetUniformLocation(shaprog1_id, "location");
	locid1_scenedepth = glGetUniformLocation(shaprog1_id, "scenedepth");
	locid1_framesize  = glGetUniformLocation(shaprog1_id, "framesize");

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenTextures(1, &texid_fbomask);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fbomask, 0
	);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaprog2_id = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/selectionvolume2.frag.glsl"
	);

	locid2_scenebeauty	= glGetUniformLocation(shaprog2_id, "scenebeautytex");
	locid2_mask 		= glGetUniformLocation(shaprog2_id, "masktex");

	radius = r > 0 ? r : SELECTIONSPHERE_DEFRADIUS;
	center = c;
}

void SelectionSphere::render(
	Camera& cam, 
	GLuint scenefbo_id, 
	GLuint scenedepthtex,
	GLuint scenebeautytex,
	Vec2i framesize
) {
	glDisable(GL_CULL_FACE);

	glUseProgram(shaprog1_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glClear(GL_COLOR_BUFFER_BIT);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	const GLfloat il[]{6,6}; const GLfloat ol[]{6,6,6,6};
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, il);
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, ol);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform2i(locid1_framesize, framesize[0], framesize[1]);

	glUniform1f(locid1_radius, radius);

	glUniform3f(
		locid1_location,
		center[0], center[1], center[2]
	);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid1_scenedepth, 0);

	glDrawArrays(GL_PATCHES, 0, 24);

	glUseProgram(shaprog2_id);
	glBindFramebuffer(GL_FRAMEBUFFER, scenefbo_id);
	glDepthMask(GL_FALSE);

	GLenum buf[]{GL_COLOR_ATTACHMENT1};
	glDrawBuffers(1, buf);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenebeautytex);
	glUniform1i(locid2_scenebeauty, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glUniform1i(locid2_mask, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

void SelectionSphere::setframesize(Vec2i size)
{
	glBindTexture(GL_TEXTURE_2D, texid_fbomask);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED, 
		size[0], size[1], 0, 
		GL_RED, GL_UNSIGNED_BYTE, nullptr
	);
}

void SelectionSphere::computepaths(GatheredData& gd)
{
	const unsigned nthreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads(nthreads);

	const unsigned npaths = gd.pathslength.size();

	// Number of Paths Per Thread
	const unsigned nppt = npaths / nthreads;

	std::vector<std::set<unsigned>> threadselectedpaths(nthreads);

	for(unsigned ti = 0; ti < nthreads; ++ti)
	{
		threads[ti] = std::thread(
			[this, &gd, &threadselectedpaths, ti, nppt](){
				
				for(
					unsigned path_i = ti * nppt; 
					path_i < (ti + 1)*nppt - 1; 
					++path_i
				){
					const unsigned nbounces = gd.pathslength[path_i];
					const unsigned off = gd.firstbounceindexes[path_i];
					for(unsigned b_i = off; b_i < off + nbounces; b_i++)
					{
						Vec3h bounceh = gd.bouncesposition[b_i];
						Vec3f bouncef = fromVec3h(bounceh);
						
						float d = length(bouncef - center);
						if(d <= radius)
						{
							threadselectedpaths[ti].insert(path_i);
						}
					
					}
				}
			}
		);
	}

	for(std::thread& th : threads)
	{
		th.join();
	}

	for(std::set<unsigned> paths : threadselectedpaths)
	{
		gd.addpaths(paths);
	}

}