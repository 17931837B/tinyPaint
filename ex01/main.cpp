#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ImageData.hpp"

GLuint		texId;
GLuint		fboId;
ImageData*	globalImg = nullptr;
GLFWwindow*	window;

// ウィンドウサイズが変更されたときに呼び出され、描画領域とアスペクト比を適切に調整する
void	framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
	float	textureAspect;
	float	windowAspect;
	float	orthoWidth = 1.0f;
	float	orthoHeight = 1.0f;

	std::cout << "Callback received width: " << width << ", height: " << height << std::endl;
	// ビューポートの定義
	glViewport(0, 0, width, height);
	// 投影行列選択
	glMatrixMode(GL_PROJECTION);
	// 単位行列にリセット
	glLoadIdentity();
	textureAspect = (float)globalImg->getWidth() / (float)globalImg->getHeight();
	windowAspect = (float)width / (float)height;
	std::cout << "windowAspect " <<windowAspect << " textureAspect " <<textureAspect << std::endl;
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
	// 投影行列の描写空間定義
	gluOrtho2D(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// キー入力のコールバック関数
void	key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void LoadTexture()
{
	globalImg = new ImageData(4096, 4096);
	// テクスチャオブジェクトを1つ生成
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	// 拡大・縮小フィルタ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// テクスチャを作成
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, globalImg->getWidth(), globalImg->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, globalImg->getImageData());
	// フレームバッファオブジェクトの生成
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: FBO" << std::endl;
	glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
	// 初期化で赤だからいらないといえばいらない
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fboId);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
}

void	display() 
{
	glClear(GL_COLOR_BUFFER_BIT); 
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
	window = glfwCreateWindow(4096, 4096, "TinyPaint ex01", NULL, NULL); 
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
	glfwTerminate();
	return (0);
}