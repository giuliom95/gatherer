#include "pathsrenderer.hpp"

void PathsRenderer::init()
{
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	glGenBuffers(1, &vboidx);
	glEnableVertexAttribArray(0);
	LOG(info) << "Created VAO";
	
	paths_number = 0;

	renderdata.disk_load_all("../data/renderdata");

	//disk_load_all_paths("../data/renderdata");
	
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
		paths_firsts.data(),
		paths_lenghts.data(),
		paths_number
	);
}

void PathsRenderer::pathsbouncinginsphere(Vec3f center, float radius)
{
	std::vector<Path*> selectedpaths;
	LOG(info) << "==START==";
	for(PathsGroup& pg : renderdata.pathgroups)
	{
		for(Path& path : pg)
		{
			LOG(info) << "** path **";
			for(Vec3h bounceh : path.points)
			{
				Vec3f bouncef = fromVec3h(bounceh);
				float d = length(bouncef - center);
				if(d <= radius)
				{
					selectedpaths.push_back(&path);
					break;
				}
			}
		}
	}
	LOG(info) << "==END==";

	paths_number = selectedpaths.size();

	paths_firsts  = std::vector<GLint>(paths_number);
	paths_lenghts = std::vector<GLsizei>(paths_number);

	GLint off = 0; 
	for(unsigned i = 0; i < paths_number; ++i)
	{
		unsigned npoints = selectedpaths[i]->points.size();
		paths_firsts[i] = off;
		paths_lenghts[i] = npoints;
		off += npoints;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboidx);

	unsigned totalbytes = off * sizeof(Vec3h);
	glBufferData(GL_ARRAY_BUFFER, totalbytes, NULL,  GL_DYNAMIC_DRAW);

	for(unsigned i = 0; i < paths_number; ++i)
	{
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			paths_firsts[i] * sizeof(Vec3h), 
			paths_lenghts[i] * sizeof(Vec3h), 
			selectedpaths[i]->points.data()
		);
	}
	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half), (void*)0
	);

}

void PathsRenderer::disk_load_all_paths(
	const boost::filesystem::path dirpath
) {
	const boost::filesystem::path lengths_fp = dirpath / "lengths.bin";
	const boost::filesystem::path paths_fp   = dirpath / "paths.bin";
	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream paths_ifs  (paths_fp);

	paths_number = boost::filesystem::file_size(lengths_fp);
	const uintmax_t paths_bytes   = boost::filesystem::file_size(paths_fp);

	std::vector<uint8_t> lenghts(paths_number);
	lengths_ifs.read(
		reinterpret_cast<char*>(lenghts.data()), paths_number
	);

	paths_firsts  = std::vector<GLint>  (paths_number);
	paths_lenghts = std::vector<GLsizei>(paths_number);
	uintmax_t offset = 0;
	for(uintmax_t i = 0; i < paths_number; ++i)
	{
		uint8_t len = lenghts[i];
		paths_firsts[i] = offset;
		paths_lenghts[i] = len;
		offset += len;
	}

	// Ignoring type because this data will be passed striaght to the GPU
	std::vector<Vec3h> paths(paths_bytes);
	paths_ifs.read(reinterpret_cast<char*>(paths.data()), paths_bytes);

	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, paths_bytes, paths.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half), (void*)0
	);
	
}