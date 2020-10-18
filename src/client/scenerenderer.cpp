#include "scenerenderer.hpp"

void SceneRenderer::init(const boost::filesystem::path& path, Camera& cam)
{
	boost::filesystem::path bin_path = 
		boost::filesystem::change_extension(path, "bin");
	LOG(info) << path << " " << bin_path;

	nlohmann::json json_data;
	boost::filesystem::ifstream json_file{path};
	if(!json_file) 
	{
		LOG(fatal) <<
			"Could not open \"" << path.string() << "\"";
		throw std::runtime_error("Could not open scene file");
	}
	json_file >> json_data;
	json_file.close();

	boost::filesystem::ifstream bin_ifs{bin_path};
	const unsigned int bin_size = boost::filesystem::file_size(bin_path);
	std::vector<char> bin_data(bin_size);
	bin_ifs.read(bin_data.data(), bin_size);

	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);

	nverts = 0;
	nidxs  = 0;

	// Compute the lenghts of the buffers
	for(const nlohmann::json json_geom : json_data["geometries"]) 
	{
		for(const nlohmann::json json_buf : json_geom["buffers"])
		{
			const std::string type = json_buf["type"];
			const unsigned buf_size = json_buf["size"];
			if(type == "vertices")
			{
				nverts += buf_size / sizeof(Vec3f);
			}
			else if(type == "indices")
			{
				nidxs  += buf_size / sizeof(unsigned);
			}
		}
	}

	// Generate the buffers
	LOG(info) << "Vertices: " << nverts;
	LOG(info) << "Indexes: " << nidxs;
	std::vector<Vec3f> vertices;
	vertices.reserve(nverts);
	std::vector<unsigned> indexes;
	indexes.reserve(nidxs);

	// Fill the buffers
	unsigned vtx_byteoff = 0;
	unsigned idx_byteoff = 0;
	unsigned maxidx = 0;
	for(const nlohmann::json json_geom : json_data["geometries"]) 
	{
		unsigned curmaxidx = maxidx;
		Geometry geom;
		geom.offset = idx_byteoff;

		for(const nlohmann::json json_buf : json_geom["buffers"])
		{
			const std::string type = json_buf["type"];
			const unsigned buf_off  = json_buf["offset"];
			const unsigned buf_size = json_buf["size"];

			if(type == "vertices")
			{
				std::move(
					reinterpret_cast<Vec3f*>(bin_data.data() + buf_off),
          			reinterpret_cast<Vec3f*>(bin_data.data() + buf_off + buf_size),
          			back_inserter(vertices)
				);
				
				vtx_byteoff += buf_size;
			}
			else if(type == "indices")
			{
				for(unsigned i = 0; i < buf_size / sizeof(unsigned); ++i)
				{
					const unsigned value = *reinterpret_cast<unsigned*>(
						bin_data.data() + buf_off + i*sizeof(unsigned)
					);
					const unsigned shiftedvalue = value + curmaxidx;
					indexes.push_back(shiftedvalue);
					maxidx = max(maxidx, shiftedvalue + 1);
				}
				
				geom.count = buf_size / sizeof(unsigned);
				idx_byteoff += buf_size;
			}

		}

		nlohmann::json json_albedo = json_geom["material"]["albedo"];
		geom.albedo = Vec3f{
			json_albedo[0],
			json_albedo[1],
			json_albedo[2]
		};

		geometries.push_back(geom);
	}

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER, 
		nverts*sizeof(Vec3f), vertices.data(), 
		GL_STATIC_DRAW
	);
	glVertexAttribPointer(
		0, 3, GL_FLOAT,
		GL_FALSE, 3 * sizeof(float), NULL
	);
	glEnableVertexAttribArray(0);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER, 
		nidxs*sizeof(unsigned), indexes.data(), 
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	const nlohmann::json json_camera = json_data["camera"];
	const nlohmann::json json_eye = json_camera["eye"];
	const Vec3f cam_eye{
		json_eye[0],
		json_eye[1],
		json_eye[2]
	};
	const nlohmann::json json_look = json_camera["look"];
	const Vec3f cam_look{
		json_look[0],
		json_look[1],
		json_look[2]
	};
	cam.focus = cam_eye + cam.r*cam_look;
	Vec3f sph = cartesian2spherical(cam_look);
	LOG(info) << cam_look << " " << sph;
	cam.yaw = sph[1];
	cam.pitch = sph[2];

	shaprog1_idx = disk_load_shader_program(
		"../src/client/shaders/scene1.vert.glsl",
		"../src/client/shaders/scene1.frag.glsl"
	);

	locid1_camvpmat = glGetUniformLocation(shaprog1_idx, "vpmat");
	locid1_geomalbedo = glGetUniformLocation(shaprog1_idx, "albedo");
	locid1_eye = glGetUniformLocation(shaprog1_idx, "eye");

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenTextures(1, &texid_fboworldpos);
	glGenTextures(1, &texid_fbobeauty);
	glGenTextures(1, &texid_fbodepth);

	glBindTexture(GL_TEXTURE_2D, texid_fboworldpos);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texid_fbobeauty);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texid_fbodepth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fboworldpos, 0
	);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, texid_fbobeauty, 0
	);  
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
		GL_TEXTURE_2D, texid_fbodepth, 0
	);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaprog2_idx = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/scene2.frag.glsl"
	);

	locid2_beautytex = glGetUniformLocation(shaprog2_idx, "beautytex");
}

void SceneRenderer::render1(Camera& cam)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glUseProgram(shaprog1_idx);
	GLenum bufs[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, bufs);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	const Vec3f eye = cam.eye();
	glUniform3f(locid1_eye, eye[0], eye[1], eye[2]);

	glBindVertexArray(vaoidx);
	for(Geometry geo : geometries)
	{
		glUniform3f(
			locid1_geomalbedo, 
			geo.albedo[0], geo.albedo[1], geo.albedo[2]
		);
		glDrawElements(
			GL_TRIANGLES, 
			geo.count, 
			GL_UNSIGNED_INT, reinterpret_cast<void*>(geo.offset)
		);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::render2()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(shaprog2_idx);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid_fbobeauty);
	glUniform1i(locid2_beautytex, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SceneRenderer::setframesize(Vec2i size)
{
	glBindTexture(GL_TEXTURE_2D, texid_fboworldpos);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB32F, 
		size[0], size[1], 0, 
		GL_RGB,  GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_fbobeauty);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA32F, 
		size[0], size[1], 0, 
		GL_RGBA, GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_fbodepth);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		size[0], size[1], 0, 
		 GL_DEPTH_COMPONENT,  GL_FLOAT, nullptr
	);
}