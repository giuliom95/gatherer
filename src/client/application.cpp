#include "application.hpp"

// Returns true if a modify has been committed 
bool mouse_camera_event(
	int btn_id,
	bool& btn_pressed,
	GLFWwindow* glfw_window,
	Vec2f& cursor_old_pos,
	std::function<void(Vec2f, Camera&)> f,
	Camera& camera
) {
	const int btn_state = 
		glfwGetMouseButton(glfw_window, btn_id);

	if (btn_state == GLFW_PRESS)
	{
		Vec2f cursor_cur_pos = get_cursor_pos(glfw_window);
		if(!btn_pressed)
		{
			btn_pressed = true;
			cursor_old_pos = cursor_cur_pos;
		}

		Vec2f cursor_delta_pos = cursor_cur_pos - cursor_old_pos;

		f(cursor_delta_pos, camera);

		cursor_old_pos = cursor_cur_pos;
		return true;
	}
	else
	{
		btn_pressed = false;
		return false;
	}
}

void rotate_camera(Vec2f cursor_delta, Camera& camera)
{
	const float mult = 0.5f;
	camera.yaw   +=  mult * cursor_delta[0];
	camera.pitch +=  mult * cursor_delta[1];
	if(camera.pitch >=  90) camera.pitch =  89.9f;
	if(camera.pitch <= -90) camera.pitch = -89.9f;
}

void dolly_camera(Vec2f cursor_delta, Camera& camera)
{
	const float mult = 0.1f;
	Vec3f p{
		0, 
		0, 
		camera.r + mult*(cursor_delta[1])
	};
	camera.focus = transformPoint(camera.c2w(), p);
}

void truckboom_camera(Vec2f cursor_delta, Camera& camera)
{
	const float mult = 0.01f;
	Vec3f p{
		-mult * cursor_delta[0], 
		 mult * cursor_delta[1], 
		camera.r
	};
	camera.focus = transformPoint(camera.c2w(), p);
}




Application::Application()
{
	LOG(info) << "Starting application";

	initglfw();
	createglfwwindow();
	
	// Enable vsync
	glfwSwapInterval(1);

	bool error = !glfwCheckErrors();
	if (error) exit(1);

	LOG(info) << "Created window";

	initglew();
	
	configureogl();

	initimgui();

	//memset(scenepath, '\0', 128);
	// TODO remove
	strcpy(scenepath, "../data/sc2s/s.json");
	strcpy(datasetA.path, "../data/sc2s/rd");
	
	// TODO move 
	cursor_old_pos = get_cursor_pos(window);
	lmb_pressed = false;
	rmb_pressed = false;
	mmb_pressed = false;
	camera_key_pressed = false;
	switch_key_pressed = false;

	glGenTextures(1, &texid_final);
	glBindTexture(GL_TEXTURE_2D, texid_final);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	accountwindowresize();

	glGenFramebuffers(1, &finalfbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, finalfbo_id);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, texid_final, 0
	);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG(error) << "Final framebuffer is not complete!";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	finalshaprog_idx = disk_load_shader_program(
		"../src/client/shaders/screenquad.vert.glsl",
		"../src/client/shaders/final.frag.glsl"
	);
	locid_finaltex = glGetUniformLocation(finalshaprog_idx, "finaltex");

	activefiltertool = ActiveFilterTool::none;

	currentdataset = nullptr;
	datasetA.id = 'A';
	datasetB.id = 'B';
}

Application::~Application()
{
	LOG(info) << "Exiting";
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Application::loop()
{
	if(glfwWindowShouldClose(window)) return false;

	glfwPollEvents();

	// Bother checking events outside GUI only if scene has been loaded
	if(sceneloaded)
	{
		if(!imgui_io->WantCaptureKeyboard)
		{
			camera_key_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT);

			if(datasetA.isloaded && datasetB.isloaded)
			{
				bool pressed = glfwGetKey(window, GLFW_KEY_X);
				if(!switch_key_pressed)
				{
					if(pressed)
					{
						switchdataset();
						mustrenderviewport = true;
						switch_key_pressed = true;
					}
				}
				else 
				{
					if(!pressed)
					{
						switch_key_pressed = false;
					}
				}
			}
		}

		if(!imgui_io->WantCaptureMouse)
		{
			if(camera_key_pressed)
			{
				mustrenderviewport |= mouse_camera_event(
					GLFW_MOUSE_BUTTON_LEFT, lmb_pressed,
					window, cursor_old_pos, rotate_camera, camera
				);

				mustrenderviewport |= mouse_camera_event(
					GLFW_MOUSE_BUTTON_RIGHT, rmb_pressed,
					window, cursor_old_pos, dolly_camera, camera
				);

				mustrenderviewport |= mouse_camera_event (
					GLFW_MOUSE_BUTTON_MIDDLE, mmb_pressed,
					window, cursor_old_pos, truckboom_camera, camera
				);

				//if(mustrenderviewport)
				//	LOG(info) << "!!! camera key + mouse";
			}
			else
			{
				// Events on path filters matter only when a dataset is loaded
				if(currentdataset != nullptr)
				{
					const int btn_state = 
						glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

					if (btn_state == GLFW_PRESS)
					{
						// Find out world position
						glBindFramebuffer(
							GL_FRAMEBUFFER, scenerenderer.opaquefbo_id
						);
						Vec2f p = get_cursor_pos(window);
						float data[4];
						glReadPixels(
							(int)p[0],(int)(framesize[1]-p[1]), 1, 1, 
							GL_RGBA, GL_FLOAT, 
							&data
						);
						Vec3f clicked_worldpoint{data[0], data[1], data[2]};
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						// Perfect zero happens only when out of scene
						if(length(clicked_worldpoint) != 0)
						{
							if(!lmb_pressed)
							{
								// Diameter = 1/10 of scene maximum length
								const float reasonabledim = 
									scenerenderer.bbox.maxlength() / 10 / 2;
								if(activefiltertool == ActiveFilterTool::sphere)
								{
									std::shared_ptr<Filter> ss(
										new SphereFilter(
											clicked_worldpoint,
											reasonabledim
										)
									);
									ss->setframesize(framesize);
									filtermanager.addfilter(ss);

									activefiltertool = ActiveFilterTool::none;
								}
								else if(activefiltertool == ActiveFilterTool::window)
								{
									Vec3f n = normalize(camera.eye() - camera.focus);
									std::shared_ptr<WindowFilter> ss(
										new WindowFilter(
											clicked_worldpoint, n, 
											{reasonabledim,reasonabledim}
										)
									);
									ss->setframesize(framesize);
									filtermanager.addfilter(ss);

									activefiltertool = ActiveFilterTool::none;
								}
								

								lmb_pressed = true;

								mustrenderviewport = true;
								//LOG(info) << "!!! click on scene";
							}
						}
					}
					else
					{
						lmb_pressed = false;
					}
				}
			}
			
		}
	}

	render();
	
	return true;
}

void Application::accountwindowresize()
{
	glfwGetFramebufferSize(window, &framesize[0], &framesize[1]);
	LOG(info) << "Framebuffer size: " << framesize;

	if(sceneloaded)
	{
		scenerenderer.setframesize(framesize);
		filtermanager.setframesize(framesize);
	}

	if(datasetA.isloaded) datasetA.pathsrenderer.setframesize(framesize);
	if(datasetB.isloaded) datasetB.pathsrenderer.setframesize(framesize);

	glBindTexture(GL_TEXTURE_2D, texid_final);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA32F, 
		framesize[0], framesize[1], 0, 
		GL_RGBA, GL_FLOAT, nullptr
	);

	camera.aspect = (float)framesize[0] / framesize[1];

	mustrenderviewport = true;
	//LOG(info) << "!!! window resized";
}

void Application::render()
{
	glViewport(0, 0, framesize[0], framesize[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(sceneloaded)
	{
		if(alwaysrenderviewport) mustrenderviewport = true;

		if(mustrenderviewport)
		{
			scenerenderer.render1(camera, true);
			scenerenderer.render1(camera, false);

			scenerenderer.render2(finalfbo_id);
			scenerenderer.render3(finalfbo_id, texid_final);
			
			if(
				currentdataset != nullptr &&  
				currentdataset->pathsrenderer.enablerendering &&
				currentdataset->gathereddata.selectedpaths.size() > 0
			) {
				currentdataset->pathsrenderer.render(
					camera, finalfbo_id,
					scenerenderer.texid_opaquebeauty, 
					scenerenderer.texid_transparentbeauty, 
					scenerenderer.texid_opaquedepth, 
					scenerenderer.texid_transparentdepth, 
					currentdataset->gathereddata
				);
			}
			
			filtermanager.render(
				camera, finalfbo_id, 
				scenerenderer.texid_opaquedepth,
				texid_final
			);
			
			mustrenderviewport = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(finalshaprog_idx);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texid_final);
		glUniform1i(locid_finaltex, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		axesvisualizer.render(camera);
		
		if(currentdataset != nullptr)
		{
			currentdataset->imagerenderer.render();
		}
	}

	renderui();

	glfwSwapBuffers(window);
}

void Application::renderui()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Resource loading");
		boost::filesystem::path cwd = boost::filesystem::current_path();
		ImGui::TextWrapped("cwd: %s", cwd.c_str());

		ImGui::Text("Scene:");
			ImGui::SameLine();
		ImGui::InputText("##scene", scenepath, 128);
		ImGui::SameLine();
		if(ImGui::Button("Load##loadscene"))
		{
			loadscene();
		}

		// User shouldn't be able to load datasets without loading a scene first
		if(sceneloaded)
		{
			ImGui::Text("Dataset A:");
			ImGui::SameLine();
			ImGui::InputText("##A", datasetA.path, 128);
			ImGui::SameLine();
			if(ImGui::Button("Load##loaddatasetA"))
			{
				loaddataset(datasetA);
			}

			if(datasetA.isloaded)
			{
				ImGui::Text("Dataset B:");
				ImGui::SameLine();
				ImGui::InputText("##B", datasetB.path, 128);
				ImGui::SameLine();
				if(ImGui::Button("Load"))
				{
					loaddataset(datasetB);
				}
			}
		}
	ImGui::End();

	// Dataset switching makes sense only when both datasets are loaded
	if(datasetA.isloaded && datasetB.isloaded)
	{
		ImGui::SetNextWindowSize({0,0});
		ImGui::Begin("Dataset switcher");
			ImGui::Text(
				"Current dataset: %c", currentdataset->id);
			if(ImGui::Button("Switch dataset"))
			{
				switchdataset();
			}
		ImGui::End();
	}

	if(sceneloaded)
	{
		ImGui::SetNextWindowSize({0,0});
		ImGui::Begin("Axes", nullptr);
			ImGui::Image(
				(void*)(intptr_t)axesvisualizer.fbotex_id, 
				{AXESVISUZLIZER_W, AXESVISUZLIZER_H},
				{0,1}, {1,0}
			);
		ImGui::End();

		ImGui::Begin("Visualization options");
			if(ImGui::CollapsingHeader("Scene"))
			{
				/*
				mustrenderviewport |= ImGui::Checkbox(
					"Backface culling", &scenerenderer.enableculling
				);
				*/
				mustrenderviewport |= ImGui::ColorEdit3(
					"Blend color", 
					reinterpret_cast<float*>(&scenerenderer.blend_color)
				);
				mustrenderviewport |= ImGui::SliderFloat(
					"Blend alpha",
					&scenerenderer.blend_alpha,
					0, 1
				);

				mustrenderviewport |= scenerenderer.renderui();
			}
			
			if(currentdataset != nullptr)
			{
				DataSet& otherdataset = 
					currentdataset->id == datasetA.id ? datasetB : datasetA;

				if(ImGui::CollapsingHeader("Paths"))
				{
					mustrenderviewport |= ImGui::Checkbox(
						"Render", &(currentdataset->pathsrenderer.enablerendering)
					);
					otherdataset.pathsrenderer.enablerendering
						= currentdataset->pathsrenderer.enablerendering;

					if(currentdataset->pathsrenderer.enablerendering)
					{
						mustrenderviewport |= ImGui::SliderFloat(
							"Paths alpha", &(currentdataset->pathsrenderer.pathsalpha), 
							0, 1
						);
						otherdataset.pathsrenderer.pathsalpha =
							currentdataset->pathsrenderer.pathsalpha;

						mustrenderviewport |= ImGui::Checkbox(
							"Depth test", &(currentdataset->pathsrenderer.enabledepth)
						);
						otherdataset.pathsrenderer.enabledepth = 
							currentdataset->pathsrenderer.enabledepth;

						mustrenderviewport |= ImGui::Checkbox(
							"Radiance scaling", 
							&(currentdataset->pathsrenderer.enableradiance)
						);
						otherdataset.pathsrenderer.enableradiance = 
							currentdataset->pathsrenderer.enableradiance;
					}
					//if(mustrenderviewport)
					//	LOG(info) << "!!! path option changed";
				}
			}
		ImGui::End();
	}

	if(currentdataset != nullptr)
	{
		DataSet& otherdataset = 
			currentdataset->id == datasetA.id ? datasetB : datasetA;

		ImGui::SetNextWindowSize({0,0});
		ImGui::Begin("Render", nullptr);
			ImGui::Image(
				(void*)(intptr_t)currentdataset->imagerenderer.fbotex_id, 
				{
					(float)currentdataset->imagerenderer.rendersize[0], 
					(float)currentdataset->imagerenderer.rendersize[1]
				}
			);

			ImGui::ColorEdit3(
				"Background color", 
				(float*)&(currentdataset->imagerenderer.bgcolor)
			);
			otherdataset.imagerenderer.bgcolor = 
				currentdataset->imagerenderer.bgcolor;

			ImGui::Combo(
				"Display mode", 
				(int*)&(currentdataset->imagerenderer.displaymode),
				"Unfiltered rendered image\0Final radiance\0Paths per pixel"
			);
			otherdataset.imagerenderer.displaymode = 
				currentdataset->imagerenderer.displaymode;
			
			if(
				currentdataset->imagerenderer.displaymode == 
				ImageDisplayMode::fullrender ||
				currentdataset->imagerenderer.displaymode == 
				ImageDisplayMode::finalradiance
			){
				ImGui::SliderFloat(
					"Exposure", &(currentdataset->imagerenderer.exposure), -2, 10
				);
				otherdataset.imagerenderer.exposure =
					currentdataset->imagerenderer.exposure;
			}

		ImGui::End();

		ImGui::Begin("Filters");
			if(ImGui::Button("Update paths"))
			{
				updateselectedpaths();
			}

			if(activefiltertool == ActiveFilterTool::none)
			{
				if(ImGui::Button("Add sphere"))
				{
					activefiltertool = ActiveFilterTool::sphere;
				}
				ImGui::SameLine();
				if(ImGui::Button("Add window"))
				{
					activefiltertool = ActiveFilterTool::window;
				}
			}
			else if(activefiltertool == ActiveFilterTool::sphere)
			{
				ImGui::Text("Placing sphere...");
			}
			else if(activefiltertool == ActiveFilterTool::window)
			{
				ImGui::Text("Placing window...");
			}

			if(filtermanager.renderui())
			{
				mustrenderviewport = true;
				//LOG(info) << "!!! filter manager UI updated";
			}
		ImGui::End();
	}

	//Debug window start
	if(sceneloaded)
	{
		ImGui::Checkbox("Always re-render", &alwaysrenderviewport);
		if(ImGui::CollapsingHeader("Camera controls"))
		{
			ImGui::DragFloat3(
				"Focus point", 
				reinterpret_cast<float*>(&(camera.focus))
			);
			ImGui::DragFloat("Pitch", &(camera.pitch));
			ImGui::DragFloat("Yaw", &(camera.yaw));
			ImGui::DragFloat("R", &(camera.r));
			ImGui::DragFloat("FOV", &(camera.fov));
			ImGui::DragFloat("Near plane", &(camera.znear));
			ImGui::DragFloat("Far plane", &(camera.zfar));
			if(ImGui::CollapsingHeader("View matrix"))
			{
				Mat4f w2c{camera.w2c()};
				char fmt[]{"%.02f"};
				ImGui::Columns(4, "w2c");
				ImGui::Text(fmt, w2c(0,0));
				ImGui::Text(fmt, w2c(0,1));
				ImGui::Text(fmt, w2c(0,2));
				ImGui::Text(fmt, w2c(0,3));
				ImGui::NextColumn();
				ImGui::Text(fmt, w2c(1,0));
				ImGui::Text(fmt, w2c(1,1));
				ImGui::Text(fmt, w2c(1,2));
				ImGui::Text(fmt, w2c(1,3));
				ImGui::NextColumn();
				ImGui::Text(fmt, w2c(2,0));
				ImGui::Text(fmt, w2c(2,1));
				ImGui::Text(fmt, w2c(2,2));
				ImGui::Text(fmt, w2c(2,3));
				ImGui::NextColumn();
				ImGui::Text(fmt, w2c(3,0));
				ImGui::Text(fmt, w2c(3,1));
				ImGui::Text(fmt, w2c(3,2));
				ImGui::Text(fmt, w2c(3,3));
			}
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::initglfw()
{
	const int glfw_init_status = glfwInit();
	if(glfw_init_status != GLFW_TRUE)
	{
		LOG(error) << "Impossible to init GLFW";
		exit(1);
	}
	else
	{
		LOG(info) << "Initialized GLFW";
	}
}

void Application::createglfwwindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(DEF_WINDOW_W, DEF_WINDOW_H, "Gatherer", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, Application::windowresize);
}

void Application::initglew()
{
	GLenum glew_init_status = glewInit();
	if (glew_init_status != GLEW_OK)
	{
		const GLubyte* err = glewGetErrorString(glew_init_status);
		LOG(error) << "GLEW: " << err;
	}
}

void Application::configureogl()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glCullFace(GL_BACK);
}

void Application::initimgui()
{
	ImGui::CreateContext();
	imgui_io = &(ImGui::GetIO());
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Application::updateselectedpaths()
{
	if(datasetA.isloaded)
	{
		filtermanager.computepaths(datasetA.gathereddata);
		datasetA.imagerenderer.updatepathmask(datasetA.gathereddata);
		datasetA.pathsrenderer.updaterenderlist(datasetA.gathereddata);
	}

	if(datasetB.isloaded)
	{
		filtermanager.computepaths(datasetB.gathereddata);
		datasetB.imagerenderer.updatepathmask(datasetB.gathereddata);
		datasetB.pathsrenderer.updaterenderlist(datasetB.gathereddata);
	}

	mustrenderviewport = true;
	//LOG(info) << "!!! path selection";
}

void Application::loadscene()
{
	scenerenderer = SceneRenderer();
	scenerenderer.init(scenepath, camera);

	axesvisualizer = AxesVisualizer();
	axesvisualizer.init();

	sceneloaded = true;
	mustrenderviewport = true;
	//LOG(info) << "!!! scene loaded";
	accountwindowresize();
}

void Application::loaddataset(DataSet& dataset)
{
	// Clean start
	dataset.gathereddata = GatheredData();
	dataset.pathsrenderer = PathsRenderer();
	dataset.imagerenderer = ImageRenderer();

	dataset.gathereddata.loadall(dataset.path, scenepath);
	dataset.pathsrenderer.init();
	dataset.imagerenderer.init(dataset.gathereddata);

	dataset.pathsrenderer.setframesize(framesize);

	currentdataset = &dataset;

	dataset.isloaded = true;
	mustrenderviewport = true;
	//LOG(info) << "!!! dataset loaded";
	LOG(info) << "Done loading dataset";
}

void Application::switchdataset()
{
	DataSet* other = 
		currentdataset->id == datasetA.id ? &datasetB : &datasetA;
	LOG(info) << "Switching from " << currentdataset->id << " to dataset " << other->id;
	currentdataset = other;
}

void Application::windowresize(GLFWwindow* window, int width, int height)
{
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	app->accountwindowresize();
	LOG(info) << "New window size: " << width << ", " << height;
}