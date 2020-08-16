#include "scenerenderer.hpp"

#include "json.hpp"

void SceneRenderer::init()
{
	boost::filesystem::path json_path =
		"/home/gmartell/Development/bouncer/scenes/simplecube/simplecube.json";

	boost::filesystem::path bin_path = json_path;
	bin_path.replace_extension(".bin");

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

	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);

	boost::filesystem::ifstream bin_ifs{bin_path};
	const unsigned int bin_size = boost::filesystem::file_size(bin_path);
	std::vector<char> bin_data(bin_size);
	bin_ifs.read(bin_data.data(), bin_size);

	for(const nlohmann::json json_geom : json_data["geometries"]) 
	{
		for(const nlohmann::json json_buf : json_geom["buffers"])
		{
			std::string type = json_buf["type"];
			unsigned int buf_size = json_buf["size"];
			unsigned int buf_off  = json_buf["offset"];
			if(type == "vertices")
			{
				glGenBuffers(1, &vboidx);
				glBindBuffer(GL_ARRAY_BUFFER, vboidx);
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
				BOOST_LOG_TRIVIAL(info) << "Loading vertices";
			}
			else if(type == "indices")
			{
				glGenBuffers(1, &eboidx);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboidx);
				glBufferData(
					GL_ELEMENT_ARRAY_BUFFER, 
					buf_size, bin_data.data() + buf_off, 
					GL_STATIC_DRAW
				);
				BOOST_LOG_TRIVIAL(info) << "Loading indices";
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 

	shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/scene.vert.glsl",
		"../src/client/shaders/scene.frag.glsl"
	);

	locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");
}

void SceneRenderer::render(Camera& cam)
{
	glUseProgram(shaprog_idx);
	glBindVertexArray(vaoidx);
	glEnable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}