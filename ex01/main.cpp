#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

GLuint	texId;
GLuint	fboId;

struct 	ImageData
{
	int				width;
	int				height;
	unsigned char*	imageData;
};

ImageData*	globalImg = nullptr;
GLFWwindow*	window; 

ImageData* createRedImage(int width, int height)
{
	ImageData*	img;
	int			i;

	img = new ImageData();
	img->width = width;
	img->height = height;
	img->imageData = new unsigned char[width * height * 4];
	// i = 0;
	// while (i < width * height)
	// {
	// 	img->imageData[i * 4 + 0] = 255;
	// 	img->imageData[i * 4 + 1] = 0;
	// 	img->imageData[i * 4 + 2] = 0;
	// 	img->imageData[i * 4 + 3] = 255;
	// 	i++;
	// }
	return (img);
}

void	framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	float	textureAspect;
	float	windowAspect;
	float	orthoWidth = 1.0f;
	float	orthoHeight = 1.0f;

	std::cout << "Callback received width: " << width << ", height: " << height << std::endl;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	textureAspect = (float)globalImg->width / (float)globalImg->height;
	windowAspect = (float)width / (float)height;
	std::cout << "win " <<windowAspect << " tex " <<textureAspect << std::endl;
	if (windowAspect > textureAspect)
	{
		orthoWidth = windowAspect / textureAspect;
		orthoHeight = 1.0f;
	}
	else
	{
		orthoWidth = 1.0f;
		orthoHeight = textureAspect / windowAspect;
	}
	gluOrtho2D(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void otherInit(void)
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f); 
	glEnable(GL_DEPTH_TEST); //深度テスト手前のみを描写
}

void LoadTexture()
{
	int	width;
	int	height;

	globalImg = createRedImage(4096, 4096);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, globalImg->width, globalImg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, globalImg->imageData);
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Framebuffer is not complete!" << std::endl;
	glViewport(0, 0, globalImg->width, globalImg->height);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glfwGetFramebufferSize(window, &width, &height); 
	glViewport(0, 0, width, height);
	glDeleteFramebuffers(1, &fboId);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
}

void	display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, texId);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, 0.f);
	glTexCoord2f(1.f, 0.f); glVertex3f( 1.0f, -1.0f, 0.f);
	glTexCoord2f(1.f, 1.f); glVertex3f( 1.0f,  1.0f, 0.f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f,  1.0f, 0.f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

int main(int argc, char **argv)
{
    int initialWidth;
    int initialHeight;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return (-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); 
    window = glfwCreateWindow(4096, 4096, "OpenGL Exercise 01", NULL, NULL); 
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return (-1);
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE; 
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return (1);
    }
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
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
        delete[] globalImg->imageData;
        delete globalImg;
        globalImg = nullptr;
    }
    if (texId)
    {
        glDeleteTextures(1, &texId);
        texId = 0;
    }
    glfwTerminate();
    return (0);
}