#include "pathsrenderer.hpp"

void PathsRenderer::init()
{
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	LOG(info) << "Created VAO";
	
	glGenBuffers(1, &vboidx);
	disk_load_all_paths("../data/renderdata");
	LOG(info) << "Loaded vertices on GPU";

	shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/paths.vert.glsl",
		"../src/client/shaders/paths.frag.glsl"
	);

	locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");

	pathsalpha = PATHSRENDERER_DEFPATHSALPHA;
	locid_pathsalpha = glGetUniformLocation(shaprog_idx, "pathsalpha");
	locid_enabledepth = glGetUniformLocation(shaprog_idx, "enabledepth");
	locid_scenedepth = glGetUniformLocation(shaprog_idx, "scenedepth");
}

void PathsRenderer::render(Camera& cam, GLuint scenedepthtex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(shaprog_idx);
	glBindVertexArray(vaoidx);
	glLineWidth(1);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glUniform1f(locid_pathsalpha, pathsalpha);
	glUniform1i(locid_enabledepth, enabledepth);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scenedepthtex);
	glUniform1i(locid_scenedepth, 0);

	glMultiDrawArrays(
		GL_LINE_STRIP, 
		scene_info.paths_firsts.data(),
		scene_info.paths_lenghts.data(),
		scene_info.paths_number
	);
}

void PathsRenderer::disk_load_all_paths(
	const boost::filesystem::path dirpath
) {
	const boost::filesystem::path lengths_fp = dirpath / "lengths.bin";
	const boost::filesystem::path paths_fp   = dirpath / "paths.bin";
	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream paths_ifs  (paths_fp);

	scene_info.paths_number = boost::filesystem::file_size(lengths_fp);
	const uintmax_t paths_bytes   = boost::filesystem::file_size(paths_fp);

	std::vector<uint8_t> lenghts(scene_info.paths_number);
	lengths_ifs.read(
		reinterpret_cast<char*>(lenghts.data()), scene_info.paths_number
	);

	scene_info.paths_firsts  = std::vector<GLint>  (scene_info.paths_number);
	scene_info.paths_lenghts = std::vector<GLsizei>(scene_info.paths_number);
	uintmax_t offset = 0;
	for(uintmax_t i = 0; i < scene_info.paths_number; ++i)
	{
		uint8_t len = lenghts[i];
		scene_info.paths_firsts[i] = offset;
		scene_info.paths_lenghts[i] = len;
		offset += len;
	}

	// Ignoring type because this data will be passed striaght to the GPU
	std::vector<Vec3h> paths(paths_bytes);
	paths_ifs.read(reinterpret_cast<char*>(paths.data()), paths_bytes);

	Vec3h minp, maxp;
	for(Vec3h v : paths)
	{
		minp[0] = min(minp[0], v[0]);
		minp[1] = min(minp[1], v[1]);
		minp[2] = min(minp[2], v[2]);
		maxp[0] = max(maxp[0], v[0]);
		maxp[1] = max(maxp[1], v[1]);
		maxp[2] = max(maxp[2], v[2]);
	}
	scene_info.bounding_box = AABB{fromVec3h(minp), fromVec3h(maxp)};
	
	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, paths_bytes, paths.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half), (void*)0
	);
	glEnableVertexAttribArray(0);
}