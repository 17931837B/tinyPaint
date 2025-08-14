#include "paint.hpp"

GLFWwindow* window;

int main(void)
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
	window = glfwCreateWindow(800, 600, "TinyPaint", NULL, NULL); 
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	otherInit();
	LoadTexture();
	glfwGetFramebufferSize(window, &initialWidth, &initialHeight);
	framebuffer_size_callback(window, initialWidth, initialHeight);
	
	std::cout << "Controls:" << std::endl;
	std::cout << "=== Colors ===" << std::endl;
	std::cout << "0: White (Eraser - overwrites with background)" << std::endl;
	std::cout << "1: Black" << std::endl;
	std::cout << "R: Red" << std::endl;
	std::cout << "G: Green" << std::endl;
	std::cout << "B: Blue" << std::endl;
	std::cout << "=== Brush Size ===" << std::endl;
	std::cout << "2-8: Direct size selection (5px - 120px)" << std::endl;
	std::cout << "Current brush size: " << brushSize << "px" << std::endl;
	std::cout << "=== Drawing ===" << std::endl;
	std::cout << "Left mouse drag: Draw" << std::endl;
	std::cout << "=== Undo/Redo ===" << std::endl;
	std::cout << "Ctrl+Z: Undo" << std::endl;
	std::cout << "Ctrl+Shift+Z or Ctrl+Y: Redo" << std::endl;
	std::cout << "=== Save ===" << std::endl;
	std::cout << "S: Save image to PNG file" << std::endl;
	std::cout << "X: Save image to XPM file" << std::endl;
	
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanupUndoSystem();
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