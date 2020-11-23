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
	locid1_showheatmap = glGetUniformLocation(shaprog1_idx, "showheatmap");
	locid1_heatmapmax = glGetUniformLocation(shaprog1_idx, "heatmapmax");
	locid1_heatmaptex = glGetUniformLocation(shaprog1_idx, "heatmap");

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

	modified |= ImGui::Checkbox("Heatmap", &showheatmap);
	if(showheatmap)
	{
		modified |= ImGui::DragFloat("Heatmap max", &heatmapmax);
	}

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
				modified = true;
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid_opaquedepth);
	glUniform1i(locid1_opaquedepth, 0);
	
	glUniform1i(locid1_showheatmap, showheatmap);
	glUniform1f(locid1_heatmapmax, heatmapmax);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texid_heatmap);
	glUniform1i(locid1_heatmaptex, 1);

	glBindVertexArray(vaoidx);
	int id = -1;
	for(Geometry& geo : geometries)
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

		glDrawArrays(GL_TRIANGLES, geo.offset, geo.count);
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

void SceneRenderer::generateheatmap(GatheredData& gd)
{
	// Dump texture from GPU
	const unsigned nuv = numuvsets;
	std::vector<Vec3f> tex(texres*texres*nuv);
	LOG(info) << "Allocated memory";
	glBindTexture(GL_TEXTURE_2D_ARRAY, texid_heatmap);
	LOG(info) << "Binded texture array";
	glGetTexImage(
		GL_TEXTURE_2D_ARRAY, 0,
		GL_RGB, GL_FLOAT,
		tex.data()
	);
	LOG(info) << "Read texture array";

	const unsigned nthreads = std::thread::hardware_concurrency();
	// Rows Per Thread
	const unsigned rpt = texres / nthreads;
	std::vector<std::thread> threads(nthreads);

	const float r = 0.707106781f * uvscalefactor;
	const float r2 = r*r;

	for(unsigned ti = 0; ti < nthreads; ++ti)
	{
		threads[ti] = std::thread(
			[nuv, r2, ti, rpt, &tex, &gd](){
				for(
					unsigned x = ti*rpt; 
					x < (ti+1)*rpt; 
					++x
				) {
					LOG(info) << "Column #" << x;
					for(unsigned y = 0; y < nuv*texres; ++y)
					{
						const unsigned idx = x + y*texres;
						Vec3f& pix = tex[idx];
						Vec3f wp = tex[idx];

						if(length(wp) != 0)
						{
							pix[0] = 0; pix[1] = 0; pix[2] = 0;
							// Iterate over bounces
							Vec3f d; float dl2;
							for(Vec3h& b : gd.bouncesposition)
							{
								d[0] = wp[0]-b[0];
								d[1] = wp[1]-b[1];
								d[2] = wp[2]-b[2];
								dl2 = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];

								if(r2 > dl2)
								{
									pix[0] += 1.0f;
								}
							}
							//LOG(info) << x << ", " << y << ": " << wp << " -> " << pix[0];
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

	// Find max for color mapping
	for(unsigned x = 0; x < texres; ++x)
		for(unsigned y = 0; y < numuvsets*texres; ++y)
		{
			const float v = tex[x + y*texres][0];
			//LOG(info) << "(" << x << ", " << y << "): " << v;
			heatmapmax = max(heatmapmax, v);
		}

	
	glTexSubImage3D(
		GL_TEXTURE_2D_ARRAY, 0, 
		0, 0, 0, texres, texres, numuvsets,
		GL_RGB, GL_FLOAT, tex.data()
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
				
				unsigned count = buf_size / sizeof(unsigned);
				geom.count = count;
				idx_byteoff += count;
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

	std::vector<Vec2f> uvs = generateuvmap(vertices, indexes);

	for(Vec3f vtx : vertices)
	{
		bbox.addpt(vtx);
	}

	std::vector<float> finalvertices;
	finalvertices.reserve(5*nidxs);

	for(unsigned i = 0; i < nidxs; ++i)
	{
		const Vec3f& v = vertices[indexes[i]];
		const Vec2f& uv = uvs[i];
		finalvertices.push_back(v[0]);
		finalvertices.push_back(v[1]);
		finalvertices.push_back(v[2]);
		finalvertices.push_back(uv[0]);
		finalvertices.push_back(uv[1]);
		//LOG(info) << v;
		//LOG(info) << uv;
	}

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER, 
		nidxs*(sizeof(Vec3f)+sizeof(Vec2f)), finalvertices.data(), 
		GL_STATIC_DRAW
	);
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, 
		sizeof(Vec3f)+sizeof(Vec2f), NULL
	);
	glVertexAttribPointer(
		1, 2, GL_FLOAT, GL_FALSE, 
		sizeof(Vec3f)+sizeof(Vec2f), (void*)(sizeof(Vec3f))
	);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	generateuvworldtextures();

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


unsigned squarify(
	std::vector<Triangle>& triangles,
	const float multiplier
) {
	const float mult = multiplier;

	//Start from texel center
	Vec2f off{0.5f, 0.5f};
	float rowheight = 0;
	unsigned rowfirstti = 0;
	unsigned uvset = 0;
	for(unsigned ti = 0; ti < triangles.size(); ++ti)
	{
		Triangle& t = triangles[ti];

		//Scaled triangle vectors must lie on texel intersections
		t.v1 = roundv(mult * t.v1);
		t.v2 = roundv(mult * t.v2);

		float width = max(t.v1[0], t.v2[0]);
		float height = max(t.v1[1], t.v2[1]);

		rowheight = max(rowheight, height);

		// if triangle goes beyond right texture border
		if(off[0] + width > texres)
		{
			// New row
			//LOG(info) << "ROW (" << rowfirstti << "/" << ti - 1 << ")";
			//LOG(info) << off[1] << " " << rowheight;
			
			// Check if new texture
			if(off[1] + rowheight > (uvset+1)*texres)
			{
				//new texture
				++uvset;
				// Move prev row to new uv set
				off[1] = texres*uvset + 0.5f;
				//LOG(info) << "SET (" << uvset << "): " << off[1];
				for(unsigned rti = rowfirstti; rti < ti; ++rti)
				{
					//LOG(info) << "rti: " << rti;
					Triangle& rt = triangles[rti];
					rt.o[1] = off[1];
				}
			}

			//normal new row
			off[0] = 0.5f;
			off[1] += rowheight + 1.0f;
			rowheight = height;
			rowfirstti = ti;
		}
		
		// Still on same row
		t.o = off;
		off[0] += width + 1.0f;
	}

	/*
	std::ofstream ofs("./uvdata");
	for(Triangle& t : triangles)
	{
		const Vec3f p0{t.o[0], 0, t.o[1]};
		const Vec3f p1{t.o[0]+t.v1[0], 0, t.o[1]+t.v1[1]};
		const Vec3f p2{t.o[0]+t.v2[0], 0, t.o[1]+t.v2[1]};
		ofs << "pmc.polyCreateFacet(p=[" << p0 << "," << p1 << "," << p2 << "])" << std::endl;
	}
	ofs.close();
	*/
	const unsigned nuvsets = uvset + 1;
	return nuvsets;
}

std::vector<Vec2f> SceneRenderer::generateuvmap(
	const std::vector<Vec3f>& vertices, 
	const std::vector<unsigned>& indexes
) {
	const unsigned nverts = indexes.size();
	std::vector<Triangle> tris;
	float totalarea = 0;

	for(unsigned tvi = 0; tvi < nverts; tvi += 3)
	{
		//LOG(info) << tvi;
		//LOG(info) << indexes[tvi + 0];
		//LOG(info) << indexes[tvi + 1];
		//LOG(info) << indexes[tvi + 2];
		const Vec3f v0 = vertices[indexes[tvi + 0]];
		const Vec3f v1 = vertices[indexes[tvi + 1]];
		const Vec3f v2 = vertices[indexes[tvi + 2]];
		//LOG(info) << v0;
		//LOG(info) << v1;
		//LOG(info) << v2;

		const Vec3f a = v1 - v0;
		const Vec3f b = v2 - v0;
		const Vec3f na = normalize(a);
		const Vec3f nb = normalize(b);
		const Vec3f n = normalize(cross(na, nb));
		const Vec3f c = cross(n, na);

		//LOG(info) << a;
		//LOG(info) << c;
		//LOG(info) << n;

		// n contains a NaN, so degenerate triangle
		if(length(n) != length(n))
		{
			//LOG(info) << "Discarded";
			continue;
		}

		const Mat4f r = transpose({na, c, n, {}});

		const Vec3f at = transformPoint(r, a);
		const Vec3f bt = transformPoint(r, b);

		Triangle t{{}, {at[1],at[0]}, {bt[1], bt[0]}, tvi};

		if(t.v2[1] < 0)
		{
			Vec2f v1{t.v2[0], -t.v2[1]};
			Vec2f v2{t.v2[0], t.v1[1]-t.v2[1]};
			t.v1 = v1;
			t.v2 = v2;
		}

		totalarea += abs(t.v1[0]*t.v2[1] - t.v1[1]*t.v2[0]);

		tris.push_back(t);
	}

	// Sort by height
	std::sort(tris.begin(), tris.end(), [](
		const Triangle& a, const Triangle& b
	){
		return max(a.v1[1], a.v2[1]) > max(b.v1[1], b.v2[1]);
	});

	const float maxheight = max(tris[0].v1[1], tris[0].v2[1]);
	uvscalefactor = texres / maxheight / 2.7f;
	numuvsets = squarify(tris, uvscalefactor);

	//create a uv buffer
	std::vector<Vec2f> uvs(nverts, Vec2f());
	
	const float invtexres = 1.0f / texres;
	for(Triangle& t : tris)
	{
		uvs[t.fvi+0] = invtexres*(t.o);
		uvs[t.fvi+1] = invtexres*(t.o + t.v1);
		uvs[t.fvi+2] = invtexres*(t.o + t.v2);
		//LOG(info) << uvs[t.fvi+0] << " " << uvs[t.fvi+1] << " " << uvs[t.fvi+2];
	}

	
	std::ofstream ofs("./uvmap");
	for(unsigned i = 0; i < uvs.size(); i += 3)
	{
		Vec3f v0{uvs[i+0][0], 0, uvs[i+0][1]};
		Vec3f v1{uvs[i+1][0], 0, uvs[i+1][1]};
		Vec3f v2{uvs[i+2][0], 0, uvs[i+2][1]};
		ofs << "pmc.polyCreateFacet(p=[" << v0 << "," << v1 << "," << v2 << "])" << std::endl;
	}
	ofs.close();
	LOG(info) << "Done outing uv map";
	
	/*
	std::ofstream ofs("./uvmap_raw");
	for(unsigned i = 0; i < uvs.size(); ++i)
		ofs << uvs[i] << std::endl;
	ofs.close();
	*/

	return uvs;
}

void SceneRenderer::generateuvworldtextures()
{
	//Generate fbo
	GLuint fboid;
	glGenFramebuffers(1, &fboid);
	glBindFramebuffer(GL_FRAMEBUFFER, fboid);

	//generate textures
	glGenTextures(1, &texid_heatmap);
	glBindTexture(GL_TEXTURE_2D_ARRAY,texid_heatmap);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB32F, texres, texres, numuvsets);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//load shaders
	GLuint sha_idx = disk_load_shader_program(
		"../src/client/shaders/uvworld.vert.glsl",
		"../src/client/shaders/uvworld.frag.glsl"
	);
	GLuint locid_uvset = glGetUniformLocation(sha_idx, "currentuvset");

	//Render
	glViewport(0, 0, texres, texres);
	glUseProgram(sha_idx);
	glBindVertexArray(vaoidx);

	// Conservative rendering
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);

	for(unsigned i = 0; i < numuvsets; ++i)
	{
		glFramebufferTextureLayer(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			texid_heatmap, 0, i
		);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOG(error) << "Heatmap framebuffer is not complete!";
		}

		GLenum bufs = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &bufs);

		glUniform1i(locid_uvset, i);
		glClear(GL_COLOR_BUFFER_BIT);
		for(Geometry& geo : geometries)
		{
			glDrawArrays(GL_TRIANGLES, geo.offset, geo.count);
		}
	}

	glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}