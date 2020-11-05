#include "scenerenderer.hpp"

void SceneRenderer::init(const boost::filesystem::path& path, Camera& cam)
{
	loadscene(path, cam);

	glGenTextures(1, &texid_fboworldposid);
	glBindTexture(GL_TEXTURE_2D, texid_fboworldposid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	generateopaquefbo();
	generatetransparentfbo();

	shaprog1_idx = disk_load_shader_program(
		"../src/client/shaders/scene1.vert.glsl",
		"../src/client/shaders/scene1.frag.glsl"
	);

	locid1_camvpmat = glGetUniformLocation(shaprog1_idx, "vpmat");
	locid1_geocolor = glGetUniformLocation(shaprog1_idx, "color");
	locid1_geomalpha = glGetUniformLocation(shaprog1_idx, "alpha");
	locid1_eye = glGetUniformLocation(shaprog1_idx, "eye");
	locid1_blend = glGetUniformLocation(shaprog1_idx, "blend");
	locid1_geomid = glGetUniformLocation(shaprog1_idx, "geomid");
	locid1_opaquedepth = glGetUniformLocation(shaprog1_idx, "opaquedepth");
	locid1_highlight = glGetUniformLocation(shaprog1_idx, "highlight");


	shaprog2_idx = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/scene2.frag.glsl"
	);
	locid2_opaquebeauty = glGetUniformLocation(shaprog2_idx, "opaquebeauty");

	shaprog3_idx = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/scene3.frag.glsl"
	);
	locid3_finaltex = glGetUniformLocation(shaprog3_idx, "finaltex");
	locid3_transparentbeauty = 
		glGetUniformLocation(shaprog3_idx, "transparentbeauty");
}

bool SceneRenderer::renderui()
{
	bool modified = false;

	if(ImGui::CollapsingHeader("Geometries"))
	{
		if(ImGui::Button("Toggle visibility for all"))
		{
			for(Geometry& g : geometries)
				g.visible = visibilitytoggle;
			visibilitytoggle = !visibilitytoggle;
			modified = true;
		}
		if(ImGui::Button("Toggle backfaces for all"))
		{
			for(Geometry& g : geometries)
				g.backfaceculling = backfacestoggle;
			backfacestoggle = !backfacestoggle;
			modified = true;
		}
		ImGui::Separator();
		unsigned idx = 0;
		for(Geometry& g : geometries)
		{
			ImGui::PushID(idx);
			if(g.visible)
			{
				if(ImGui::Button("Hide"))
				{
					g.visible = false;
					modified = true;
				}
			}
			else
			{
				if(ImGui::Button("Show"))
				{
					g.visible = true;
					modified = true;
				}
			}
			ImGui::SameLine();

			if(g.backfaceculling)
			{
				if(ImGui::Button("Show back"))
				{
					g.backfaceculling = false;
					modified = true;
				}
			}
			else
			{
				if(ImGui::Button("Hide back"))
				{
					g.backfaceculling = true;
					modified = true;
				}
			}
			ImGui::SameLine();


			if(ImGui::Selectable(g.name.c_str(), selected_geom == (int)idx))
			{
				selected_geom = idx;
			}
			ImGui::PopID();
			++idx;
		}
	}
	return modified;
}

void SceneRenderer::render1(Camera& cam, bool opaque)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER, 
		opaque ? opaquefbo_id : transparentfbo_id
	);

	glUseProgram(shaprog1_idx);

	GLenum bufs2clear[]{GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT0};
	glDrawBuffers(opaque ? 2 : 1, bufs2clear);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum bufs[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, bufs);
	glEnable(GL_DEPTH_TEST);

	const Mat4f vpmat = cam.w2c()*cam.persp();
	glUniformMatrix4fv(
		locid1_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	const Vec3f eye = cam.eye();
	glUniform3f(locid1_eye, eye[0], eye[1], eye[2]);

	glUniform4f(
		locid1_blend, 
		blend_color[0], blend_color[1], blend_color[2], blend_alpha
	);

	if(!opaque)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texid_opaquedepth);
		glUniform1i(locid1_opaquedepth, 0);
	}

	glBindVertexArray(vaoidx);
	int id = -1;
	for(Geometry geo : geometries)
	{
		++id;
		if(!geo.visible) continue;

		if( opaque && geo.alpha <  1) continue;
		if(!opaque && geo.alpha == 1) continue;
		
		glUniform1f(locid1_geomalpha, geo.alpha);

		glUniform1i(locid1_geomid, id);

		glUniform1i(locid1_highlight, id == selected_geom ? 1 : 0);

		glUniform3f(
			locid1_geocolor, 
			geo.color[0], geo.color[1], geo.color[2]
		);

		if(geo.backfaceculling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		glDrawElements(
			GL_TRIANGLES, 
			geo.count, 
			GL_UNSIGNED_INT, reinterpret_cast<void*>(geo.offset)
		);

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_CULL_FACE);
}

void SceneRenderer::render2(GLuint finalfbo)
{
	glBindFramebuffer(GL_FRAMEBUFFER, finalfbo);
	glUseProgram(shaprog2_idx);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid_opaquebeauty);
	glUniform1i(locid2_opaquebeauty, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SceneRenderer::render3(GLuint finalfbo, GLuint finaltex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, finalfbo);
	glUseProgram(shaprog3_idx);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finaltex);
	glUniform1i(locid3_finaltex, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid_transparentbeauty);
	glUniform1i(locid3_transparentbeauty, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void SceneRenderer::setframesize(Vec2i size)
{
	glBindTexture(GL_TEXTURE_2D, texid_fboworldposid);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA32F, 
		size[0], size[1], 0, 
		GL_RGBA, GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_opaquebeauty);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA8, 
		size[0], size[1], 0, 
		GL_RGBA, GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_opaquedepth);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		size[0], size[1], 0, 
		GL_DEPTH_COMPONENT,  GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_transparentbeauty);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA8, 
		size[0], size[1], 0, 
		GL_RGBA,  GL_FLOAT, nullptr
	);

	glBindTexture(GL_TEXTURE_2D, texid_transparentdepth);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		size[0], size[1], 0, 
		GL_DEPTH_COMPONENT,  GL_FLOAT, nullptr
	);
}

void SceneRenderer::loadscene(const boost::filesystem::path& path, Camera& cam)
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
	unsigned idx = 0;
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
		nlohmann::json json_material = json_geom["material"];

		nlohmann::json json_albedo = json_material["albedo"];
		Vec3f albedo{
			json_albedo[0],
			json_albedo[1],
			json_albedo[2]
		};

		nlohmann::json json_emission = json_material["emission"];
		Vec3f emission{
			json_emission[0],
			json_emission[1],
			json_emission[2]
		};

		geom.color = albedo + emission;

		nlohmann::json json_opacity = json_material["opacity"];
		nlohmann::json json_translucency = json_material["translucency"];
		nlohmann::json json_transmission = json_material["transmission"];
		float opacity = 
			json_opacity.is_null() ? 1 : (float)json_opacity;
		float translucency = 
			json_translucency.is_null() ? 0 : (float)json_translucency;
		float transmission = 
			json_transmission.is_null() ? 0 : (float)json_transmission;
		
		if(opacity < 1 || translucency > 0 || transmission > 0)
			geom.alpha = 0.7f;
		else
			geom.alpha = 1;

		nlohmann::json json_name = json_geom["name"];
		geom.name = 
			json_name.is_null() ? "Geometry " + idx : (std::string)json_name;

		geometries.push_back(geom);
	}

	for(Vec3f vtx : vertices)
	{
		bbox.addpt(vtx);
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

	//cam.r = length(cam_eye - bbox.center());
	cam.focus = cam_eye + cam.r*cam_look;
	Vec3f sph = cartesian2spherical(cam_look);
	LOG(info) << cam_look << " " << sph;
	cam.yaw = sph[1];
	cam.pitch = sph[2];
}

void SceneRenderer::generateopaquefbo()
{
	glGenFramebuffers(1, &opaquefbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, opaquefbo_id);

	glGenTextures(1, &texid_opaquebeauty);
	glGenTextures(1, &texid_opaquedepth);

	glBindTexture(GL_TEXTURE_2D, texid_opaquebeauty);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texid_opaquedepth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fboworldposid, 0
	);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, texid_opaquebeauty, 0
	);  
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
		GL_TEXTURE_2D, texid_opaquedepth, 0
	);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG(error) << "Opaque framebuffer is not complete!";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::generatetransparentfbo()
{
	glGenFramebuffers(1, &transparentfbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, transparentfbo_id);

	glGenTextures(1, &texid_transparentbeauty);
	glGenTextures(1, &texid_transparentdepth);

	glBindTexture(GL_TEXTURE_2D, texid_transparentbeauty);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texid_transparentdepth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setframesize({DEF_WINDOW_W, DEF_WINDOW_H});

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_fboworldposid, 0
	);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, texid_transparentbeauty, 0
	);  
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
		GL_TEXTURE_2D, texid_transparentdepth, 0
	);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG(error) << "Transparent framebuffer is not complete!";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}