#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

	const char *vtxsha_src = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	GLuint vtxsha_idx = glCreateShader(GL_VERTEX_SHADER);
	BOOST_LOG_TRIVIAL(info) << "Created vertex shader";
	glShaderSource(vtxsha_idx, 1, &vtxsha_src, NULL);
	glCompileShader(vtxsha_idx);
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
	BOOST_LOG_TRIVIAL(info) << "Compiled fragment shader";

	GLuint shaprog_idx;
	shaprog_idx = glCreateProgram();
	glAttachShader(shaprog_idx, vtxsha_idx);
	glAttachShader(shaprog_idx, fragsha_idx);
	glLinkProgram(shaprog_idx);
	glUseProgram(shaprog_idx);
	glDeleteShader(vtxsha_idx);
	glDeleteShader(fragsha_idx);
	BOOST_LOG_TRIVIAL(info) << "Created shader program";


	const float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};
	GLuint vaoidx;
	glGenVertexArrays(1, &vaoidx);
	glBindVertexArray(vaoidx);
	BOOST_LOG_TRIVIAL(info) << "Created VAO";

	GLuint vboidx;
	glGenBuffers(1, &vboidx);
	glBindBuffer(GL_ARRAY_BUFFER, vboidx);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	BOOST_LOG_TRIVIAL(info) << "Loaded vertices on GPU";


	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}