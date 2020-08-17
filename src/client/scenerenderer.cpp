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

		unsigned vtxbuf_off  = 0;
		unsigned idxbuf_off  = 0;
		unsigned nvtx = 0;

		for(const nlohmann::json json_buf : json_geom["buffers"])
		{
			std::string type = json_buf["type"];
			unsigned int buf_off  = json_buf["offset"];
			unsigned int buf_size = json_buf["size"];
			if(type == "vertices")
			{
				vtxbuf_off  = buf_off;
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
				nvtx = buf_size / 12;
				BOOST_LOG_TRIVIAL(info) << "Loaded vertices";
			}
			else if(type == "indices")
			{
				idxbuf_off  = buf_off;
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

		std::vector<Vec3f> normals(nvtx);
		for(Vec3f& n : normals) n = Vec3f();

		unsigned ntris = (geom.nelems / 3);
		// Compute normals
		for(unsigned ti = 0; ti < ntris; ++ti)
		{
			const uint32_t i0 = *(uint32_t*)&(bin_data[4*(3*ti+0) + idxbuf_off]);
			const uint32_t i1 = *(uint32_t*)&(bin_data[4*(3*ti+1) + idxbuf_off]);
			const uint32_t i2 = *(uint32_t*)&(bin_data[4*(3*ti+2) + idxbuf_off]);
			const Vec3f v0 = *(Vec3f*)&(bin_data[i0*3*4 + vtxbuf_off]);
			const Vec3f v1 = *(Vec3f*)&(bin_data[i1*3*4 + vtxbuf_off]);
			const Vec3f v2 = *(Vec3f*)&(bin_data[i2*3*4 + vtxbuf_off]);

			const Vec3f v01 = v1 - v0;
			const Vec3f v02 = v2 - v0;
			const Vec3f n = normalize(cross(v01, v02));
			normals[i0] = normals[i0] + n;
			normals[i1] = normals[i1] + n;
			normals[i2] = normals[i2] + n;
		}

		for(Vec3f& n : normals) n = normalize(n);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER, 
			nvtx*sizeof(Vec3f), normals.data(), 
			GL_STATIC_DRAW
		);
		glVertexAttribPointer(
			1, 3, GL_FLOAT,
			GL_FALSE, 3 * sizeof(float), (void*)0
		);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
		geometries.push_back(geom);
		BOOST_LOG_TRIVIAL(info) << "Loaded geometry";
	}

	shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/scene.vert.glsl",
		"../src/client/shaders/scene.frag.glsl"
	);

	locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");
	locid_geomcolor = glGetUniformLocation(shaprog_idx, "color");
}

void SceneRenderer::render(Camera& cam)
{
	glUseProgram(shaprog_idx);
	glEnable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	for(Geometry geo : geometries)
	{
		glBindVertexArray(geo.vaoidx);
		glUniform3f(locid_geomcolor, geo.color[0], geo.color[1], geo.color[2]);
		glDrawElements(GL_TRIANGLES, geo.nelems, GL_UNSIGNED_INT, NULL);
	}
}