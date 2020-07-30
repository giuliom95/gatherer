#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "utils.hpp"
#include "gatherer.hpp"
#include "camera.hpp"

#include "pathsrenderer.hpp"
#include "axesvisualizer.hpp"

#include <boost/log/trivial.hpp>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#define WINDOW_W 1024
#define WINDOW_H 1024

bool glfwCheckErrors()
{
	const char* err_msg;
	int err_code = glfwGetError(&err_msg);
	if(err_code != GLFW_NO_ERROR)
	{
		BOOST_LOG_TRIVIAL(error) << "GLFW: " << err_msg;
		return false;
	}
	return true;
}

Vec2f get_cursor_pos(GLFWwindow* window)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return {(float)x, (float)y};
}

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
	camera.pitch += -0.5f * cursor_delta[1];
}

void dolly_camera(Vec2f cursor_delta, Camera& camera)
{
	float mult = .1*log(.3*camera.r + 2);
	camera.r +=  mult * cursor_delta[0];
	camera.r +=  mult * cursor_delta[1];
	if (camera.r < 1) camera.r = 1;
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

void render_all(
	GLFWwindow*		window,
	Camera&			camera,
	PathsRenderer&	pathsrenderer,
	AxesVisualizer& axesviz
) {
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	pathsrenderer.render(camera);
	axesviz.render(camera);

	ImGui::SetNextWindowSize({0,0});
	ImGui::Begin(
		"Axes", nullptr, 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar
	);
	ImGui::Image(
		(void*)(intptr_t)axesviz.fbotex_id, 
		{AXESVISUZLIZER_W, AXESVISUZLIZER_H},
		{0,1}, {1,0}
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

	glfwSwapBuffers(window);
}

int main()
{
	
	GLFWwindow* glfw_window;

	const int glfw_init_status = glfwInit();
	if(glfw_init_status != GLFW_TRUE)
	{
		BOOST_LOG_TRIVIAL(error) << "Impossible to init GLFW";
		exit(1);
	} else
		BOOST_LOG_TRIVIAL(info) << "Initialized GLFW";

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfw_window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Gatherer", NULL, NULL);
	glfwMakeContextCurrent(glfw_window);

	glfwCheckErrors();
	BOOST_LOG_TRIVIAL(info) << "Created window";

	GLenum glew_init_status = glewInit();
	if (glew_init_status != GLEW_OK)
	{
		const GLubyte* err = glewGetErrorString(glew_init_status);
		BOOST_LOG_TRIVIAL(error) << "GLEW: " << err;
	}
	
	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	PathsRenderer pathsrenderer;
	pathsrenderer.init();

	AxesVisualizer axesvisualizer;
	axesvisualizer.init();

	// Set camera focus to bbox center
	Vec3f center = pathsrenderer.scene_info.bounding_box.center();

	Camera cam;
	cam.focus = center;
	cam.pitch = 0;
	cam.yaw = 0;
	cam.r = 20;

	cam.znear = 0;
	cam.zfar  = 2000;
	cam.fov   = 10;

	ImGui::CreateContext();
	ImGuiIO& imgui_io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init("#version 450 core");
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, false);

	Vec2f cursor_old_pos = get_cursor_pos(glfw_window);
	bool lmb_pressed = false;
	bool rmb_pressed = false;
	bool mmb_pressed = false;

	// First render to show something on screen on startup
	render_all(glfw_window, cam, pathsrenderer, axesvisualizer);

	while (!glfwWindowShouldClose(glfw_window))
	{
		glfwWaitEvents();

		if(!imgui_io.WantCaptureMouse)
		{
			mouse_camera_event(
				GLFW_MOUSE_BUTTON_LEFT,
				lmb_pressed,
				glfw_window,
				cursor_old_pos,
				rotate_camera, cam
			);

			mouse_camera_event(
				GLFW_MOUSE_BUTTON_RIGHT,
				rmb_pressed,
				glfw_window,
				cursor_old_pos,
				dolly_camera, cam
			);

			mouse_camera_event(
				GLFW_MOUSE_BUTTON_MIDDLE,
				mmb_pressed,
				glfw_window,
				cursor_old_pos,
				truckboom_camera, cam
			);
		}

		render_all(glfw_window, cam, pathsrenderer, axesvisualizer);

	}
	
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;	
}