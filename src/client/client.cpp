#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "gatherer.hpp"

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

std::vector<uint8_t> 
disk_load_all_paths(
	const GLuint					vboidx,
	const boost::filesystem::path	dirpath
)
{
	const boost::filesystem::path lengths_fp = dirpath / "lengths.bin";
	const boost::filesystem::path paths_fp   = dirpath / "paths.bin";
	boost::filesystem::ifstream lengths_ifs(lengths_fp);
	boost::filesystem::ifstream paths_ifs  (paths_fp);

	const uintmax_t lengths_bytes = boost::filesystem::file_size(lengths_fp);
	const uintmax_t paths_bytes   = boost::filesystem::file_size(paths_fp);

	std::vector<uint8_t> lengths(lengths_bytes);
	lengths_ifs.read(reinterpret_cast<char*>(lengths.data()), lengths_bytes);

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
	

	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, paths_bytes, paths.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(
		0, 3, GL_HALF_FLOAT, 
		GL_FALSE, 3 * sizeof(half_float::half), (void*)0
	);
	glEnableVertexAttribArray(0);

	return lengths;
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

	Mat4f view()
	{
		const float rad_pitch = pitch * PI_OVER_180;
		const float rad_yaw   = yaw   * PI_OVER_180;
		const float a = r * cosf(rad_pitch);
		const Vec3f lpos{
			a*cosf(rad_yaw), 
			r*sinf(rad_pitch), 
			a*sinf(rad_yaw)
		};
		const Vec3f pos = lpos + focus;
		BOOST_LOG_TRIVIAL(info) << pos[0] << " " << pos[1] << " " << pos[2];
		const Vec3f z = normalize(-1*lpos);
		const Vec3f up{0,1,0};
		const Vec3f x = normalize(cross(up, z));
		const Vec3f y = cross(z, x);
		const Mat4f mrot{x, y, z, {}};
		const Mat4f mpos{{1,0,0},{0,1,0},{0,0,1}, -1*pos};
		return mpos*transpose(mrot);
	}

	Mat4f persp()
	{
		return {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, .01, 0,
			0, 0, 0, 1
		};
	}
};

Vec2f get_cursor_pos(GLFWwindow* window)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return {(float)x, (float)y};
}

int main(void)
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
	const std::vector<uint8_t> lengths = disk_load_all_paths(
		vboidx, "../data/renderdata"
	);
	BOOST_LOG_TRIVIAL(info) << "Loaded vertices on GPU";

	GLuint shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/pathvertex.glsl",
		"../src/client/shaders/pathfragment.glsl"
	);
	glUseProgram(shaprog_idx);

	GLint mat_locid = glGetUniformLocation(shaprog_idx, "mvp");

	glEnablei(GL_BLEND, 0);
	glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	Camera cam;
	cam.focus = Vec3f{};
	cam.pitch = 0;
	cam.yaw = 0;
	cam.r = 20;

	Vec2f cursor_old_pos = get_cursor_pos(glfw_window);
	bool lmb_pressed = false;
	bool rmb_pressed = false;

	while (!glfwWindowShouldClose(glfw_window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		const Mat4f m = cam.view() * cam.persp();
		glUniformMatrix4fv(
			mat_locid, 1, GL_FALSE, 
			reinterpret_cast<const GLfloat*>(&m)
		);

		GLint off = 0;
		for(const uint8_t len : lengths)
		{
			glDrawArrays(GL_LINE_STRIP, off, len);
			off += len;
		}
		glfwSwapBuffers(glfw_window);

		glfwWaitEvents();

		const int glfw_lmb_state = 
			glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT);

		if (glfw_lmb_state == GLFW_PRESS)
		{
			Vec2f cursor_cur_pos = get_cursor_pos(glfw_window);
			if(!lmb_pressed)
			{
				lmb_pressed = true;
				cursor_old_pos = cursor_cur_pos;
			}

			Vec2f cursor_delta_pos = cursor_cur_pos - cursor_old_pos;

			cam.yaw += 0.1*cursor_delta_pos[0];
			cam.pitch += -0.1*cursor_delta_pos[1];
			BOOST_LOG_TRIVIAL(info) << cam.yaw << " " << cam.pitch;

			cursor_old_pos = cursor_cur_pos;
		}
		else
		{
			lmb_pressed = false;
		}

		const int glfw_rmb_state = 
			glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT);

		if (glfw_rmb_state == GLFW_PRESS)
		{
			Vec2f cursor_cur_pos = get_cursor_pos(glfw_window);
			if(!rmb_pressed)
			{
				rmb_pressed = true;
				cursor_old_pos = cursor_cur_pos;
			}

			Vec2f cursor_delta_pos = cursor_cur_pos - cursor_old_pos;

			cam.r += cursor_delta_pos[1];
			BOOST_LOG_TRIVIAL(info) << cam.r;

			cursor_old_pos = cursor_cur_pos;
		}
		else
		{
			rmb_pressed = false;
		}
	}
	
	glfwTerminate();
	return 0;
	
}