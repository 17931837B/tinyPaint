#include "paint.hpp"

GLFWwindow* window;

int	main(void)
{
	int	initialWidth;
	int	initialHeight;

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return (-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); 
	window = glfwCreateWindow(800, 600, "TinyPaint ex04", NULL, NULL); 
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return (-1);
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE; 
	GLenum err;
	err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return (1);
	}
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	// コールバック設定
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	otherInit();
	LoadTexture();
	glfwGetFramebufferSize(window, &initialWidth, &initialHeight);
	framebuffer_size_callback(window, initialWidth, initialHeight);
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	if (globalImg)
	{
		delete globalImg;
		globalImg = nullptr;
	}
	if (texId)
	{
		glDeleteTextures(1, &texId);
		texId = 0;
	}
	if (fboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}
	glfwTerminate();
	return (0);
}
