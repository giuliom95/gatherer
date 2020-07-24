#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "gatherer.hpp"
#include <boost/log/trivial.hpp>

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

class AABB {
public:
	Vec3f min, max;
	Vec3f center() {
		return 0.5f * (min + max);
	}
};

class SceneInfo
{
public:
	std::vector<uint8_t>	path_lenghts;
	AABB					bounding_box;
};

SceneInfo disk_load_all_paths(
	const GLuint					vboidx,
	const boost::filesystem::path	dirpath
) {
	SceneInfo scene_info;

	const boost::filesystem::path lengths_fp = dirpath / "lengths.bin";
	const boost::filesystem::path paths_fp   = dirpath / "paths.bin";
	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream paths_ifs  (paths_fp);

	const uintmax_t lengths_bytes = boost::filesystem::file_size(lengths_fp);
	const uintmax_t paths_bytes   = boost::filesystem::file_size(paths_fp);

	scene_info.path_lenghts = std::vector<uint8_t>(lengths_bytes);
	lengths_ifs.read(
		reinterpret_cast<char*>(scene_info.path_lenghts.data()), lengths_bytes
	);

	// Ignoring type because this data will be passed striaght to the GPU
	std::vector<Vec3h> paths(paths_bytes);
	paths_ifs.read(reinterpret_cast<char*>(paths.data()), paths_bytes);

	Vec3h minp, maxp;
	for(Vec3h v : paths)
	{
		minp[0] = min(minp[0], v[0]);
		minp[1] = min(minp[1], v[1]);
		minp[2] = min(minp[2], v[2]);
		maxp[0] = max(maxp[0], v[0]);
		maxp[1] = max(maxp[1], v[1]);
		maxp[2] = max(maxp[2], v[2]);
	}
	scene_info.bounding_box = AABB{fromVec3h(minp), fromVec3h(maxp)};
	
	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, paths_bytes, paths.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half_float::half), (void*)0
	);
	glEnableVertexAttribArray(0);

	return scene_info;
}

GLuint disk_load_shader(
	const boost::filesystem::path&	path,
	const GLenum 					type
)
{
	GLint compile_status;
	boost::filesystem::ifstream ifs{path};
	std::string str(
		(std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
	);
	ifs.close();
	const char* src = str.c_str();
	GLuint idx = glCreateShader(type);
	glShaderSource(idx, 1, &src, nullptr);
	glCompileShader(idx);
	glGetShaderiv(idx, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE)
	{
		BOOST_LOG_TRIVIAL(error) << "Shader has errors!";
		char info[512];
		glGetShaderInfoLog(idx, 512, NULL, info);
    	BOOST_LOG_TRIVIAL(error) << info;
		return -1;
	}
	BOOST_LOG_TRIVIAL(info) << "Compiled shader";

	return idx;
}

GLuint disk_load_shader_program(
	const boost::filesystem::path& vtxsha_path,
	const boost::filesystem::path& fragsha_path
)
{
	GLuint vtxsha_idx  = disk_load_shader(vtxsha_path,  GL_VERTEX_SHADER);
	GLuint fragsha_idx = disk_load_shader(fragsha_path, GL_FRAGMENT_SHADER);

	GLuint shaprog_idx;
	shaprog_idx = glCreateProgram();
	glAttachShader(shaprog_idx, vtxsha_idx);
	glAttachShader(shaprog_idx, fragsha_idx);
	glLinkProgram(shaprog_idx);
	glDeleteShader(vtxsha_idx);
	glDeleteShader(fragsha_idx);
	BOOST_LOG_TRIVIAL(info) << "Created shader program";

	return shaprog_idx;
}

class Camera
{
public:
	Vec3f focus;
	float pitch;
	float yaw;
	float r;

	float fov;
	float zfar;
	float znear;

	Mat4f w2c()
	{
		Mat4f mrot;
		Vec3f pos;
		viewmatrices(pos, mrot);
		return translationMatrix(-1*pos)*transpose(mrot);
	}

	Mat4f c2w()
	{
		Mat4f mrot;
		Vec3f pos;
		viewmatrices(pos, mrot);
		return mrot*translationMatrix(pos);
	}

	Mat4f persp()
	{
		const float a = 1 / tanf(fov);
		const float d = zfar - znear;
		const float b = -(zfar + znear) / d;
		const float c = -2 * zfar * znear / d;
		return {
			a, 0, 0, 0,
			0, a, 0, 0,
			0, 0, b, c,
			0, 0, 1, 0
		};
	}
private:
	void viewmatrices(Vec3f& pos, Mat4f& mrot)
	{
		const float rad_pitch = pitch * PI_OVER_180;
		const float rad_yaw   = yaw   * PI_OVER_180;
		const float a = r * cosf(rad_pitch);
		const Vec3f lpos{
			a*cosf(rad_yaw), 
			-r*sinf(rad_pitch), 
			a*sinf(rad_yaw)
		};
		//BOOST_LOG_TRIVIAL(info) << pos[0] << " " << pos[1] << " " << pos[2];
		const Vec3f z = normalize(-1*lpos);
		const Vec3f up{0,1,0};
		const Vec3f x = normalize(cross(up, z));
		const Vec3f y = cross(z, x);
		mrot = Mat4f{x, y, z, {}};
		pos = lpos + focus;
	}
};

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
	camera.yaw   += -0.5f * cursor_delta[0];
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
	GLFWwindow*	window,
	Camera&		camera,
	GLint		locid_camvpmat,
	SceneInfo&	scene_info
) {
	const Mat4f vpmat = camera.w2c()*camera.persp();
	glUniformMatrix4fv(
		locid_camvpmat, 1, GL_FALSE, 
		reinterpret_cast<const GLfloat*>(&vpmat)
	);

	GLint off = 0;
	for(const uint8_t len : scene_info.path_lenghts)
	{
		glDrawArrays(GL_LINE_STRIP, off, len);
		off += len;
	}

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
	glfw_window = glfwCreateWindow(1024, 1024, "Hello World", NULL, NULL);
	glfwMakeContextCurrent(glfw_window);

	glfwCheckErrors();
	BOOST_LOG_TRIVIAL(info) << "Created window";

	GLenum glew_init_status = glewInit();
	if (glew_init_status != GLEW_OK)
	{
		const GLubyte* err = glewGetErrorString(glew_init_status);
		BOOST_LOG_TRIVIAL(error) << "GLEW: " << err;
	}

	GLuint vaoidx;
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	BOOST_LOG_TRIVIAL(info) << "Created VAO";

	GLuint vboidx;
	glGenBuffers(1, &vboidx);
	SceneInfo scene_info = disk_load_all_paths(
		vboidx, "../data/renderdata"
	);
	BOOST_LOG_TRIVIAL(info) << "Loaded vertices on GPU";

	GLuint shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/pathvertex.glsl",
		"../src/client/shaders/pathfragment.glsl"
	);
	glUseProgram(shaprog_idx);

	GLint locid_camvpmat = glGetUniformLocation(shaprog_idx, "vpmat");

	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Set camera focus to bbox center
	Vec3f center = scene_info.bounding_box.center();

	Camera cam;
	cam.focus = center;
	cam.pitch = 0;
	cam.yaw = 0;
	cam.r = 20;

	cam.znear = .1;
	cam.zfar  = 2000;
	cam.fov   = 10;

	Vec2f cursor_old_pos = get_cursor_pos(glfw_window);
	bool lmb_pressed = false;
	bool rmb_pressed = false;
	bool mmb_pressed = false;

	// First render to show something on screen on startup
	render_all(glfw_window, cam, locid_camvpmat, scene_info);

	while (!glfwWindowShouldClose(glfw_window))
	{
		glfwWaitEvents();

		bool hasToUpdate = false;

		hasToUpdate |= mouse_camera_event(
			GLFW_MOUSE_BUTTON_LEFT,
			lmb_pressed,
			glfw_window,
			cursor_old_pos,
			rotate_camera, cam
		);

		hasToUpdate |= mouse_camera_event(
			GLFW_MOUSE_BUTTON_RIGHT,
			rmb_pressed,
			glfw_window,
			cursor_old_pos,
			dolly_camera, cam
		);

		hasToUpdate |= mouse_camera_event(
			GLFW_MOUSE_BUTTON_MIDDLE,
			mmb_pressed,
			glfw_window,
			cursor_old_pos,
			truckboom_camera, cam
		);

		glClear(GL_COLOR_BUFFER_BIT);

		if(hasToUpdate)
		{
			render_all(glfw_window, cam, locid_camvpmat, scene_info);
		}
	}
	
	glfwTerminate();
	return 0;	
}