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
disk_load_all_opengl(
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
	std::vector<char> paths(paths_bytes);
	paths_ifs.read(paths.data(), paths_bytes);

	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, paths_bytes, paths.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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

int main(void)
{
	GLFWwindow* window;

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
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	glfwMakeContextCurrent(window);
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
	const std::vector<uint8_t> lengths = disk_load_all_opengl(
		vboidx, "../data/renderdata"
	);
	BOOST_LOG_TRIVIAL(info) << "Loaded vertices on GPU";

	GLuint shaprog_idx = disk_load_shader_program(
		"../src/client/shaders/pathvertex.glsl",
		"../src/client/shaders/pathfragment.glsl"
	);
	glUseProgram(shaprog_idx);



	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		for(const uint8_t len : lengths)
		{
			glDrawArrays(GL_LINE_STRIP, 0, len);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}