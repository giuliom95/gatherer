#include "scenerenderer.hpp"

#include "json.hpp"

void SceneRenderer::init()
{
	boost::filesystem::path json_path = "../data/renderdata/scene.json";
	boost::filesystem::path bin_path = "../data/renderdata/scene.bin";

	nlohmann::json json_data;
	boost::filesystem::ifstream json_file{json_path};
	if(!json_file) 
	{
		BOOST_LOG_TRIVIAL(fatal) <<
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
		geom.color = Vec3f{
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
				BOOST_LOG_TRIVIAL(info) << "Loaded vertices";
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
				BOOST_LOG_TRIVIAL(info) << "Loaded indices";
			}
		}

		glBindVertexArray(0);
		geometries.push_back(geom);
		BOOST_LOG_TRIVIAL(info) << "Loaded geometry";
	}

	shaprog1_idx = disk_load_shader_program(
		"../src/client/shaders/scene1.vert.glsl",
		"../src/client/shaders/scene1.frag.glsl"
	);

	locid1_camvpmat = glGetUniformLocation(shaprog1_idx, "vpmat");
	locid1_geomcolor = glGetUniformLocation(shaprog1_idx, "color");


	glGenFramebuffers(1, &fbo_idx);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_idx);

	glGenTextures(1, &fbocolortex_idx);
	glBindTexture(GL_TEXTURE_2D, fbocolortex_idx);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, 
		WINDOW_W, WINDOW_H, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, fbocolortex_idx, 0
	);  

	glGenTextures(1, &fbodepthtex_idx);
	glBindTexture(GL_TEXTURE_2D, fbodepthtex_idx);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		WINDOW_W, WINDOW_H, 0, 
		GL_DEPTH_COMPONENT,  GL_FLOAT, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 1);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
		GL_TEXTURE_2D, fbodepthtex_idx, 0
	);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		BOOST_LOG_TRIVIAL(error) << "Framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shaprog2_idx = disk_load_shader_program(
		"../src/client/shaders/scene2.vert.glsl",
		"../src/client/shaders/scene2.frag.glsl"
	);
}

void SceneRenderer::render1(Camera& cam)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_idx);

	glUseProgram(shaprog1_idx);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	for(Geometry geo : geometries)
	{
		glBindVertexArray(geo.vaoidx);
		glUniform3f(locid1_geomcolor, geo.color[0], geo.color[1], geo.color[2]);
		glDrawElements(GL_TRIANGLES, geo.nelems, GL_UNSIGNED_INT, NULL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::render2()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(shaprog2_idx);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}