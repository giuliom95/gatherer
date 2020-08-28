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

	BOOST_LOG_TRIVIAL(info) << "Created window";

	initglew();
	
	configureogl();

	initimgui();

	scenerenderer.init();
	axesvisualizer.init();
	selectionvolume.init();

	camera.focus = scenerenderer.bbox.center();
	camera.pitch = 0;
	camera.yaw = 0;
	camera.r = 20;

	camera.znear = 1;
	camera.zfar  = 2000;
	camera.fov   = 10;

	cursor_old_pos = get_cursor_pos(window);
	lmb_pressed = false;
	rmb_pressed = false;
	mmb_pressed = false;
	camera_key_pressed = false;
}

Application::~Application()
{
	BOOST_LOG_TRIVIAL(info) << "Exiting";
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

	if(!imgui_io->WantCaptureKeyboard)
	{
		camera_key_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
	}

	if(!imgui_io->WantCaptureMouse)
	{
		if(camera_key_pressed)
		{
			mouse_camera_event(
				GLFW_MOUSE_BUTTON_LEFT, lmb_pressed,
				window, cursor_old_pos, rotate_camera, camera
			);

			mouse_camera_event(
				GLFW_MOUSE_BUTTON_RIGHT, rmb_pressed,
				window, cursor_old_pos, dolly_camera, camera
			);

			mouse_camera_event (
				GLFW_MOUSE_BUTTON_MIDDLE, mmb_pressed,
				window, cursor_old_pos, truckboom_camera, camera
			);
		}
		else
		{
			const int btn_state = 
				glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

			if (btn_state == GLFW_PRESS)
			{
				// Find out world position
				glBindFramebuffer(GL_FRAMEBUFFER, scenerenderer.fbo_id);
				Vec2f p = get_cursor_pos(window);
				glReadPixels(
					(int)p[0],(int)(1024-p[1]), 1, 1, 
					GL_RGB, GL_FLOAT, 
					&clicked_worldpoint
				);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				if(length(clicked_worldpoint) != 0)
					selectionvolume.location = clicked_worldpoint;
			}
		}
		
	}

	render();
	
	return true;
}

void Application::render()
{
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	scenerenderer.render1(camera);
	selectionvolume.render(
		camera,  
		scenerenderer.fbo_id, 
		scenerenderer.texid_fbodepth,
		scenerenderer.texid_fbobeauty
	);
	scenerenderer.render2();
	axesvisualizer.render(camera);

	renderui();

	glfwSwapBuffers(window);
}

void Application::renderui()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize({0,0});
	ImGui::Begin(
		"Axes", nullptr, 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar
	);
		ImGui::Image(
			(void*)(intptr_t)axesvisualizer.fbotex_id, 
			{AXESVISUZLIZER_W, AXESVISUZLIZER_H},
			{0,1}, {1,0}
		);
	ImGui::End();

	/*
	ImGui::Begin("Paths options");
		ImGui::SliderFloat(
			"Paths alpha", &(pathsrenderer.pathsalpha), 0, 1
		);
		ImGui::Checkbox("Depth test", &(pathsrenderer.enabledepth));
	ImGui::End();
	*/

	ImGui::Begin("Selection volume");
		ImGui::LabelText(
			"World position",
			"%.02f, %.02f, %.02f", 
			clicked_worldpoint[0],
			clicked_worldpoint[1],
			clicked_worldpoint[2]
		);
	ImGui::End();

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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::initglfw()
{
	const int glfw_init_status = glfwInit();
	if(glfw_init_status != GLFW_TRUE)
	{
		BOOST_LOG_TRIVIAL(error) << "Impossible to init GLFW";
		exit(1);
	}
	else
	{
		BOOST_LOG_TRIVIAL(info) << "Initialized GLFW";
	}
}

void Application::createglfwwindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Gatherer", NULL, NULL);
	glfwMakeContextCurrent(window);
}

void Application::initglew()
{
	GLenum glew_init_status = glewInit();
	if (glew_init_status != GLEW_OK)
	{
		const GLubyte* err = glewGetErrorString(glew_init_status);
		BOOST_LOG_TRIVIAL(error) << "GLEW: " << err;
	}
}

void Application::configureogl()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void Application::initimgui()
{
	ImGui::CreateContext();
	imgui_io = &(ImGui::GetIO());
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
}