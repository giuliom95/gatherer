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

GLuint create_shader_program()
{
	GLint compile_status;

	const char *vtxsha_src = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	mat4 m = mat4(-4.37444448471, 0.0, 1.22467117936e-16, -1.22464679915e-16, 0.0, 4.37444448471, 0.0, 0.0, -5.35714943625e-16, 0.0, -1.00001990795, 1.0, 1216.09556675, -1194.22334433, -1186.37361909, 1186.54999929);\n"
		"	gl_Position = m*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"	gl_Position /= gl_Position.w;\n"
		"}\0";
	GLuint vtxsha_idx = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vtxsha_idx, 1, &vtxsha_src, NULL);
	glCompileShader(vtxsha_idx);
	glGetShaderiv(vtxsha_idx, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE)
	{
		BOOST_LOG_TRIVIAL(error) << "Vertex shader has errors!";
		return -1;
	}
	BOOST_LOG_TRIVIAL(info) << "Compiled vertex shader";

	const char *fragsha_src = "#version 330 core\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"	out_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";
	GLuint fragsha_idx;
	fragsha_idx = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragsha_idx, 1, &fragsha_src, NULL);
	glCompileShader(fragsha_idx);
	glGetShaderiv(fragsha_idx, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE)
	{
		BOOST_LOG_TRIVIAL(error) << "Fragment shader has errors!";
		return -1;
	}
	BOOST_LOG_TRIVIAL(info) << "Compiled fragment shader";

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

	GLuint shaprog_idx = create_shader_program();
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