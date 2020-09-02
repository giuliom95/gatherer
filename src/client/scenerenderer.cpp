#include "scenerenderer.hpp"

#include "json.hpp"

void SceneRenderer::init(Camera& cam)
{
	boost::filesystem::path json_path = "../data/renderdata/scene.json";
	boost::filesystem::path bin_path = "../data/renderdata/scene.bin";

	nlohmann::json json_data;
	boost::filesystem::ifstream json_file{json_path};
	if(!json_file) 
	{
		LOG(fatal) <<
			"Could not open \"" << json_path.string() << "\"";
		throw std::runtime_error("Could not open scene file");
	}
	json_file >> json_data;
	json_file.close();

	boost::filesystem::ifstream bin_ifs{bin_path};
	const unsigned int bin_size = boost::filesystem::file_size(bin_path);
	std::vector<char> bin_data(bin_size);
	bin_ifs.read(bin_data.data(), bin_size);

	for(const nlohmann::json json_geom : json_data["geometries"]) 
	{
		GLuint vaoidx;
		glGenVertexArrays(1, &vaoidx);
		glBindVertexArray(vaoidx);

		Geometry geom;
		geom.vaoidx = vaoidx;
		geom.nelems = 0;	// Will be filled later. If it stays zero, something went wrong.
		nlohmann::json json_albedo = json_geom["material"]["albedo"];
		geom.albedo = Vec3f{
			json_albedo[0],
			json_albedo[1],
			json_albedo[2]
		};

		for(const nlohmann::json json_buf : json_geom["buffers"])
		{
			std::string type = json_buf["type"];
			unsigned int buf_off  = json_buf["offset"];
			unsigned int buf_size = json_buf["size"];
			if(type == "vertices")
			{
				GLuint vbo;
				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(
					GL_ARRAY_BUFFER, 
					buf_size, bin_data.data() + buf_off, 
					GL_STATIC_DRAW
				);
				glVertexAttribPointer(
					0, 3, GL_FLOAT,
					GL_FALSE, 3 * sizeof(float), (void*)0
				);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				for(unsigned i = 0; i < buf_size / sizeof(Vec3f); ++i) {
					Vec3f p = *reinterpret_cast<Vec3f*>(
						bin_data.data() + buf_off + i*sizeof(Vec3f)
					);
					bbox.addpt(p);
				}

				LOG(info) << "Loaded vertices";
			}
			else if(type == "indices")
			{
				GLuint ebo;
				glGenBuffers(1, &ebo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
				glBufferData(
					GL_ELEMENT_ARRAY_BUFFER, 
					buf_size, bin_data.data() + buf_off, 
					GL_STATIC_DRAW
				);
				geom.nelems = buf_size / 4;
				LOG(info) << "Loaded indices";
			}
		}

		glBindVertexArray(0);
		geometries.push_back(geom);
		LOG(info) << "Loaded geometry";
	}

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
	glBindTexture(GL_TEXTURE_2D, texid_fboworldpos);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB32F, 
		WINDOW_W, WINDOW_H, 0, 
		GL_RGB,  GL_FLOAT, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fboworldpos, 0
	);

	glGenTextures(1, &texid_fbobeauty);
	glBindTexture(GL_TEXTURE_2D, texid_fbobeauty);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, 
		WINDOW_W, WINDOW_H, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, texid_fbobeauty, 0
	);  

	glGenTextures(1, &texid_fbodepth);
	glBindTexture(GL_TEXTURE_2D, texid_fbodepth);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		WINDOW_W, WINDOW_H, 0, 
		 GL_DEPTH_COMPONENT,  GL_FLOAT, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
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

	for(Geometry geo : geometries)
	{
		glBindVertexArray(geo.vaoidx);
		glUniform3f(locid1_geomalbedo, geo.albedo[0], geo.albedo[1], geo.albedo[2]);
		glDrawElements(GL_TRIANGLES, geo.nelems, GL_UNSIGNED_INT, NULL);
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