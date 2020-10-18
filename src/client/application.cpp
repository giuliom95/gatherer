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
	camera.yaw   +=  0.5f * cursor_delta[0];
	camera.pitch +=  0.5f * cursor_delta[1];
	if(camera.pitch >=  90) camera.pitch =  89.9f;
	if(camera.pitch <= -90) camera.pitch = -89.9f;
}

void dolly_camera(Vec2f cursor_delta, Camera& camera)
{
	float mult = 1;
	Vec3f p{
		0, 
		0, 
		camera.r + mult*(cursor_delta[1])
	};
	camera.focus = transformPoint(camera.c2w(), p);
}

void truckboom_camera(Vec2f cursor_delta, Camera& camera)
{
	float mult = 1;
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

	memset(scenepath, '\0', 128);
	
	// TODO move 
	cursor_old_pos = get_cursor_pos(window);
	lmb_pressed = false;
	rmb_pressed = false;
	mmb_pressed = false;
	camera_key_pressed = false;
	switch_key_pressed = false;

	accountwindowresize();

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
				bool pressed = glfwGetKey(window, GLFW_KEY_TAB);
				if(!switch_key_pressed)
				{
					if(pressed)
					{
						switchdataset();
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
						glBindFramebuffer(GL_FRAMEBUFFER, scenerenderer.fbo_id);
						Vec2f p = get_cursor_pos(window);
						Vec3f clicked_worldpoint;
						glReadPixels(
							(int)p[0],(int)(framesize[1]-p[1]), 1, 1, 
							GL_RGB, GL_FLOAT, 
							&clicked_worldpoint
						);
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						// Perfect zero happens only when out of scene
						if(length(clicked_worldpoint) != 0)
						{
							if(!lmb_pressed)
							{

								if(activefiltertool == ActiveFilterTool::sphere)
								{
									std::shared_ptr<Filter> ss(
										new SphereFilter(clicked_worldpoint)
									);
									ss->setframesize(framesize);
									filtermanager.addfilter(ss);

									activefiltertool = ActiveFilterTool::none;
								}
								else if(activefiltertool == ActiveFilterTool::window)
								{
									Vec3f n = normalize(camera.eye() - camera.focus);
									std::shared_ptr<WindowFilter> ss(
										new WindowFilter(clicked_worldpoint, n)
									);
									ss->setframesize(framesize);
									filtermanager.addfilter(ss);

									activefiltertool = ActiveFilterTool::none;
								}
								

								lmb_pressed = true;

								mustrenderviewport = true;
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

	scenerenderer.setframesize(framesize);
	filtermanager.setframesize(framesize);

	camera.aspect = (float)framesize[0] / framesize[1];

	mustrenderviewport = true;
}

void Application::render()
{
	glViewport(0, 0, framesize[0], framesize[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(sceneloaded)
	{
		if(mustrenderviewport)
		{
			scenerenderer.render1(camera);
			filtermanager.render(
				camera,  
				scenerenderer.fbo_id, 
				scenerenderer.texid_fbodepth,
				scenerenderer.texid_fbobeauty
			);

			if(
				currentdataset != nullptr && 
				currentdataset->pathsrenderer.enablerendering
			) {
				currentdataset->pathsrenderer.render(
					camera, scenerenderer.fbo_id,
					scenerenderer.texid_fbodepth, 
					framesize, currentdataset->gathereddata
				);
			}

			mustrenderviewport = false;
		}
		scenerenderer.render2();

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
				"Background color", (float*)&(currentdataset->imagerenderer.bgcolor)
			);
			otherdataset.imagerenderer.bgcolor = 
				currentdataset->imagerenderer.bgcolor;

			ImGui::Combo(
				"Display mode", (int*)&(currentdataset->imagerenderer.displaymode),
				"Final luminance\0Paths per pixel"
			);
			otherdataset.imagerenderer.displaymode = 
				currentdataset->imagerenderer.displaymode;
			
			if(currentdataset->imagerenderer.displaymode == finalluminance)
			{
				ImGui::SliderFloat(
					"Exposure", &(currentdataset->imagerenderer.exposure), -2, 10
				);
				otherdataset.imagerenderer.exposure =
					currentdataset->imagerenderer.exposure;
			}

		ImGui::End();

		
		ImGui::Begin("Paths options");
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
			}
		ImGui::End();
	}

	//Debug window start
	if(sceneloaded)
	{
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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Application::initimgui()
{
	ImGui::CreateContext();
	imgui_io = &(ImGui::GetIO());
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
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
}

void Application::loadscene()
{
	scenerenderer = SceneRenderer();
	scenerenderer.init(scenepath, camera);

	axesvisualizer = AxesVisualizer();
	axesvisualizer.init();

	sceneloaded = true;
	mustrenderviewport = true;
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

	currentdataset = &dataset;

	dataset.isloaded = true;
	mustrenderviewport = true;
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