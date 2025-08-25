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
	
std::cout << "\n";
std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
std::cout << "║              🎨 TinyPaint Controls           ║" << std::endl;
std::cout << "╠══════════════════════════════════════════════╣" << std::endl;
std::cout << "║  🌈 COLORS                                   ║" << std::endl;
std::cout << "║    0  ⚪ White (Eraser)                      ║" << std::endl;
std::cout << "║    1  ⚫ Black                               ║" << std::endl;
std::cout << "║    R  🔴 Red                                 ║" << std::endl;
std::cout << "║    G  🟢 Green                               ║" << std::endl;
std::cout << "║    B  🔵 Blue                                ║" << std::endl;
std::cout << "╠──────────────────────────────────────────────╢" << std::endl;
std::cout << "║  🖌️  BRUSH SIZE                               ║" << std::endl;
std::cout << "║    2-8  Direct size (5px - 120px)            ║" << std::endl;
std::cout << "║    Current: " << std::setw(3) << brushSize << "px                            ║" << std::endl;
std::cout << "╠──────────────────────────────────────────────╢" << std::endl;
std::cout << "║  🖱️  DRAWING                                  ║" << std::endl;
std::cout << "║    Left Mouse Drag  →  Draw                  ║" << std::endl;
std::cout << "╠──────────────────────────────────────────────╢" << std::endl;
std::cout << "║  ⏪ UNDO/REDO                                ║" << std::endl;
std::cout << "║    Ctrl+Z           →  Undo                  ║" << std::endl;
std::cout << "║    Ctrl+Shift+Z/Y   →  Redo                  ║" << std::endl;
std::cout << "╠──────────────────────────────────────────────╢" << std::endl;
std::cout << "║  💾 SAVE                                     ║" << std::endl;
std::cout << "║    S  →  Save as PNG                         ║" << std::endl;
std::cout << "║    X  →  Save as XPM                         ║" << std::endl;
std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
std::cout << "\n";
	
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
