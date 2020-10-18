#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "axesvisualizer.hpp"
#include "scenerenderer.hpp"
#include "pathsrenderer.hpp"
#include "imagerenderer.hpp"

#include "filtermanager.hpp"
#include "spherefilter.hpp"
#include "windowfilter.hpp"

#include "camera.hpp"

#include "gathereddata.hpp"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

enum ActiveFilterTool{
	none,
	sphere,
	window
};

class DataSet
{
public:
	DataSet()
	{
		isloaded = false;
		memset(path, '\0', 128);
	}
	GatheredData   gathereddata;
	PathsRenderer  pathsrenderer;
	ImageRenderer  imagerenderer;
	bool isloaded;
	char path[128];
};

class Application
{
public:
	Application();
	~Application();
	bool loop();

	void accountwindowresize();
private:
	bool sceneloaded = false;
	char scenepath[128];
	
	DataSet datasetA;

	GLFWwindow* window;
	Vec2i framesize;

	AxesVisualizer axesvisualizer;
	SceneRenderer  scenerenderer;

	FilterManager    filtermanager;
	ActiveFilterTool activefiltertool;

	Camera camera;
	ImGuiIO* imgui_io;

	Vec2f cursor_old_pos;
	bool lmb_pressed;
	bool rmb_pressed;
	bool mmb_pressed;
	bool camera_key_pressed;

	bool mustrenderviewport = true;

	void render();
	void renderui();

	void initglfw();
	void createglfwwindow();
	void initglew();
	void configureogl();
	void initimgui();

	void updateselectedpaths();

	void loadscene();
	void loaddataset();

	static void windowresize(GLFWwindow* window, int width, int height);
};

#endif