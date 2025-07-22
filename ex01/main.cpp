#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

GLuint  texId;
GLuint  fboId;

struct	ImageData
{
	int	width;
	int	height;
	unsigned char*	imageData;
};

ImageData*	createRedImage(int width, int height)
{
	ImageData*	img = new ImageData();
	img->width = width;
	img->height = height;
	img->imageData = new unsigned char[width * height * 4]; 
	for (int i = 0; i < width * height; ++i)
	{
		img->imageData[i * 4 + 0] = 255;
		img->imageData[i * 4 + 1] = 0;
		img->imageData[i * 4 + 2] = 0;
		img->imageData[i * 4 + 3] = 255;
	}
	return (img);
}

ImageData*	globalImg = nullptr;
GLFWwindow*	window; 

// void	framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
// 	glViewport(0, 0, width, height);

// 	glMatrixMode(GL_PROJECTION);
// 	glLoadIdentity();

// 	float textureAspect = (float)globalImg->width / (float)globalImg->height;
// 	float windowAspect = (float)width / (float)height;

//     // Adjust the orthographic projection based on window aspect ratio
//     // This ensures the texture's aspect ratio is maintained and black bars appear if needed.
//     if (windowAspect > textureAspect)
//     {
//         // Window is wider than the texture (e.g., 16:9 window, 1:1 texture)
//         // Adjust X-axis range to fit the wider window, Y-axis range remains -1.0 to 1.0
//         gluOrtho2D(-windowAspect, windowAspect, -1.0, 1.0);
//     }
//     else
//     {
//         // Window is taller than the texture (e.g., 9:16 window, 1:1 texture)
//         // Adjust Y-axis range to fit the taller window, X-axis range remains -1.0 to 1.0
//         gluOrtho2D(-1.0, 1.0, -1.0 / windowAspect, 1.0 / windowAspect);
//     }
    
//     // Switch back to Modelview Matrix mode
//     glMatrixMode(GL_MODELVIEW);
//     glLoadIdentity(); // Reset the modelview matrix
// }

// // GLFW callback function for keyboard input
// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//     // If ESC key is pressed, set the window to close
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//     {
//         glfwSetWindowShouldClose(window, GL_TRUE);
//     }
// }

// // Custom initialization for OpenGL settings (called once)
// void otherInit(void)
// {
//     // Set the clear color to black (to clearly see the aspect ratio bars)
//     glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
//     // Enable depth testing for correct 3D rendering (though not strictly needed for 2D quad)
//     glEnable(GL_DEPTH_TEST);
// }

// // Function to load and initialize the texture using an FBO
// void LoadTexture()
// {
//     // Create the red image data (4096x4096 RGBA)
//     globalImg = createRedImage(4096, 4096);

//     // --- Texture Generation and Setup ---
//     glGenTextures(1, &texId); // Generate a texture ID
//     glBindTexture(GL_TEXTURE_2D, texId); // Bind the texture

//     // Set texture filtering parameters
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Magnification filter
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Minification filter
    
//     // Upload the image data to the texture (RGBA format)
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, globalImg->width, globalImg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, globalImg->imageData);
    
//     // --- Framebuffer Object (FBO) for Texture Initialization ---
//     glGenFramebuffers(1, &fboId); // Generate an FBO ID
//     glBindFramebuffer(GL_FRAMEBUFFER, fboId); // Bind the FBO

//     // Attach the texture to the FBO's color attachment point
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

//     // Check if the FBO is complete and ready for rendering
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//     {
//         std::cerr << "Framebuffer is not complete!" << std::endl;
//         // In a real application, you might want more robust error handling here
//     }

//     // Set the viewport to the size of the texture (for rendering to the FBO)
//     glViewport(0, 0, globalImg->width, globalImg->height);

//     // Clear the FBO's attached texture with red color (as per exercise requirement)
//     glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Set clear color to red
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the FBO's color and depth buffers

//     // Unbind the FBO to return to the default framebuffer (the screen)
//     glBindFramebuffer(GL_FRAMEBUFFER, 0); 

//     // Restore the viewport to the main window's size
//     int width, height;
//     glfwGetFramebufferSize(window, &width, &height); 
//     glViewport(0, 0, width, height);

//     // Delete the FBO as it's only used for initialization in this case
//     glDeleteFramebuffers(1, &fboId);
// }

// // Main display/rendering function (called repeatedly in the main loop)
// void display() 
// {
//     // Clear the main window's color and depth buffers with the background color (black)
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
//     glLoadIdentity(); // Reset the modelview matrix

//     // Bind the texture to be drawn
//     glBindTexture(GL_TEXTURE_2D, texId);
//     glEnable(GL_TEXTURE_2D); // Enable 2D texturing

//     // Draw a quad that fills the normalized device coordinates (-1.0 to 1.0)
//     // The aspect ratio adjustment is handled by the projection matrix set in framebuffer_size_callback
//     glBegin(GL_QUADS);
//     glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, 0.f);
//     glTexCoord2f(1.f, 0.f); glVertex3f( 1.0f, -1.0f, 0.f);
//     glTexCoord2f(1.f, 1.f); glVertex3f( 1.0f,  1.0f, 0.f);
//     glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f,  1.0f, 0.f);
//     glEnd();

//     glDisable(GL_TEXTURE_2D); // Disable 2D texturing after drawing
// }

// // Main function
// int main(int argc, char **argv)
// {
//     // Initialize GLFW
//     if (!glfwInit())
//     {
//         std::cerr << "Failed to initialize GLFW" << std::endl;
//         return -1;
//     }

//     // Set OpenGL context hints for version and profile
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     // Request a Compatibility Profile, as indicated by glxinfo output
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); 

//     // Create the GLFW window
//     // Initial window size can be anything, as framebuffer_size_callback will adjust
//     window = glfwCreateWindow(4096, 6000, "OpenGL Exercise 01", NULL, NULL); 
//     if (!window)
//     {
//         std::cerr << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     // Make the created window's OpenGL context current on the calling thread
//     glfwMakeContextCurrent(window);

//     // IMPORTANT: glewExperimental must be set to GL_TRUE before glewInit()
//     // This allows GLEW to load all available modern OpenGL functions.
//     glewExperimental = GL_TRUE; 
//     // Initialize GLEW (must be called after a valid OpenGL context is made current)
//     GLenum err = glewInit();
//     if (GLEW_OK != err)
//     {
//         std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
//         glfwTerminate();
//         return 1;
//     }
//     // Print OpenGL and GLSL versions for debugging/verification
//     std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
//     std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

//     // Register GLFW callback functions
//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // For window resizing
//     glfwSetKeyCallback(window, key_callback); // For keyboard input

//     otherInit(); // Custom OpenGL initialization
//     LoadTexture(); // Load and initialize the texture using FBO

//     // Call the resize callback once at the beginning to set up initial projection
//     int initialWidth, initialHeight;
//     glfwGetFramebufferSize(window, &initialWidth, &initialHeight);
//     framebuffer_size_callback(window, initialWidth, initialHeight);

//     // Main rendering loop
//     while (!glfwWindowShouldClose(window))
//     {
//         display(); // Call the display function to render the scene
//         glfwSwapBuffers(window); // Swap the front and back buffers to show the rendered frame
//         glfwPollEvents(); // Process all pending events (e.g., input, window events)
//     }

//     // --- Resource Cleanup ---
//     // Delete the dynamically allocated image data
//     if (globalImg) {
//         delete[] globalImg->imageData;
//         delete globalImg;
//         globalImg = nullptr;
//     }
//     // Delete the OpenGL texture
//     if (texId) {
//         glDeleteTextures(1, &texId);
//         texId = 0;
//     }

//     // Terminate GLFW
//     glfwTerminate();
//     return (0);
// }