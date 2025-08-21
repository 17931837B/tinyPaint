#include "paint.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

GLuint		texId;
GLuint		fboId;
ImageData*	globalImg = nullptr;
BrushColor	currentBrushColor = {0.0f, 0.0f, 0.0f, 1.0f};
bool		isDragging = false;
double		lastMouseX = 0.0;
double		lastMouseY = 0.0;
float		brushSize = 30.0f;
int currentBrushSizeIndex = 3;

void	framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
	float	textureAspect;
	float	windowAspect;
	float	orthoWidth = 1.0f;
	float	orthoHeight = 1.0f;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	textureAspect = (float)globalImg->getWidth() / (float)globalImg->getHeight();
	windowAspect = (float)width / (float)height;
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

void screenToTexture(double screenX, double screenY, float& texX, float& texY)
{
	int	windowWidth, windowHeight;
	float	textureAspect;
	float	windowAspect;
	float drawAreaX, drawAreaY, drawAreaWidth, drawAreaHeight;
	float relativeX;
	float relativeY;

	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	textureAspect = (float)globalImg->getWidth() / (float)globalImg->getHeight();
	windowAspect = (float)windowWidth / (float)windowHeight;
	if (windowAspect > textureAspect)
	{
		drawAreaHeight = (float)windowHeight;
		drawAreaWidth = drawAreaHeight * textureAspect;
		drawAreaX = ((float)windowWidth - drawAreaWidth) * 0.5f;
		drawAreaY = 0.0f;
	} 
	else
	{
		drawAreaWidth = (float)windowWidth;
		drawAreaHeight = drawAreaWidth / textureAspect;
		drawAreaX = 0.0f;
		drawAreaY = ((float)windowHeight - drawAreaHeight) * 0.5f;
	}
	//正規化
	relativeX = (screenX - drawAreaX) / drawAreaWidth;
	relativeY = (screenY - drawAreaY) / drawAreaHeight;
	// テクスチャ座標に変換
	texX = relativeX * globalImg->getWidth();
	texY = (1.0f - relativeY) * globalImg->getHeight(); // Y軸反転
}

// 円の描写
void	drawCircle(float centerX, float centerY, float radius)
{
	int		i;
	float	angle, x, y;

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(currentBrushColor.r, currentBrushColor.g, currentBrushColor.b, currentBrushColor.a);
	glVertex2f(centerX, centerY);
	i = 0;
	while (i <= CIRCLE_SEGMENTS)
	{
		angle = 2.0f * M_PI * i / CIRCLE_SEGMENTS;
		x = centerX + radius * cos(angle);
		y = centerY + radius * sin(angle);
		glVertex2f(x, y);
		i++;
	}
	glEnd();
}

// 線の描写
void	drawLine(float x1, float y1, float x2, float y2)
{
	float	dx;
	float	dy;
	float	distance;
	int		steps;
	int		i;
	float	s, x, y;

	dx = x2 - x1;
	dy = y2 - y1;
	distance = sqrt(dx * dx + dy * dy);
	if (distance < 1.0f)
	{
		drawCircle(x2, y2, brushSize / 2.0f);
		return ;
	}
	steps = (int)(distance / (brushSize * 0.2f)) + 1;
	i = 0;
	while (i <= steps)
	{
		s = (float)i / steps;
		x = x1 + s * dx;
		y = y1 + s * dy;
		drawCircle(x, y, brushSize / 2.0f);
		i++;
	}
}

void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
	float	texX, texY;
	int		windowWidth, windowHeight;

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			isDragging = true;
			glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
			screenToTexture(lastMouseX, lastMouseY, texX, texY);
			if (texX >= 0 && texX < globalImg->getWidth() && texY >= 0 && texY < globalImg->getHeight())
			{
				glBindFramebuffer(GL_FRAMEBUFFER, fboId);
				glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluOrtho2D(0, globalImg->getWidth(), 0, globalImg->getHeight());
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				drawCircle(texX, texY, brushSize / 2.0f);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
				framebuffer_size_callback(window, windowWidth, windowHeight);
			}
		}
		else if (action == GLFW_RELEASE)
			isDragging = false;
	}
}

void	cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos)
{
	float texX1, texY1, texX2, texY2;

	if (isDragging)
	{
		screenToTexture(lastMouseX, lastMouseY, texX1, texY1);
		screenToTexture(xpos, ypos, texX2, texY2);
		if (texX1 >= 0 && texX1 < globalImg->getWidth() && texY1 >= 0 && texY1 < globalImg->getHeight() &&
			texX2 >= 0 && texX2 < globalImg->getWidth() && texY2 >= 0 && texY2 < globalImg->getHeight())
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboId);
			glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0, globalImg->getWidth(), 0, globalImg->getHeight());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			drawLine(texX1, texY1, texX2, texY2);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			int windowWidth, windowHeight;
			glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
			framebuffer_size_callback(window, windowWidth, windowHeight);
		}
		lastMouseX = xpos;
		lastMouseY = ypos;
	}
}

void	key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
			case GLFW_KEY_S:
				saveImage();
				break;
			case GLFW_KEY_0:
				currentBrushColor = {1.0f, 1.0f, 1.0f, 1.0f};
				std::cout << "Brush color: White (Eraser - overwrites with background)" << std::endl;
				break ;
			case GLFW_KEY_1:
				currentBrushColor = {0.0f, 0.0f, 0.0f, 1.0f};
				std::cout << "Brush color: Black" << std::endl;
				break ;
			case GLFW_KEY_R:
				currentBrushColor = {1.0f, 0.0f, 0.0f, 1.0f};
				std::cout << "Brush color: Red" << std::endl;
				break ;
			case GLFW_KEY_G:
				currentBrushColor = {0.0f, 1.0f, 0.0f, 1.0f};
				std::cout << "Brush color: Green" << std::endl;
				break ;
			case GLFW_KEY_B:
				currentBrushColor = {0.0f, 0.0f, 1.0f, 1.0f};
				std::cout << "Brush color: Blue" << std::endl;
				break ;
			case GLFW_KEY_KP_SUBTRACT:
				if (currentBrushSizeIndex > 0)
				{
					currentBrushSizeIndex--;
					brushSize = BRUSH_SIZES[currentBrushSizeIndex];
					std::cout << "Brush size: " << brushSize << "px" << std::endl;
				}
				break ;
			case GLFW_KEY_KP_ADD:
				if (currentBrushSizeIndex < NUM_BRUSH_SIZES - 1)
				{
					currentBrushSizeIndex++;
					brushSize = BRUSH_SIZES[currentBrushSizeIndex];
					std::cout << "Brush size: " << brushSize << "px" << std::endl;
				}
				break ;
			case GLFW_KEY_2:
				currentBrushSizeIndex = 0;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (smallest)" << std::endl;
				break ;
			case GLFW_KEY_3:
				currentBrushSizeIndex = 1;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (small)" << std::endl;
				break ;
			case GLFW_KEY_4:
				currentBrushSizeIndex = 2;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (medium-small)" << std::endl;
				break ;
			case GLFW_KEY_5:
				currentBrushSizeIndex = 3;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (medium)" << std::endl;
				break ;
			case GLFW_KEY_6:
				currentBrushSizeIndex = 4;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (large)" << std::endl;
				break ;
			case GLFW_KEY_7:
				currentBrushSizeIndex = 5;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (very large)" << std::endl;
				break ;
			case GLFW_KEY_8:
				currentBrushSizeIndex = 6;
				brushSize = BRUSH_SIZES[currentBrushSizeIndex];
				std::cout << "Brush size: " << brushSize << "px (largest)" << std::endl;
				break ;
		}
	}
}

void otherInit(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 背景は白
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void LoadTexture()
{
	int	width;
	int	height;

	globalImg = new ImageData(4096, 4096);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, globalImg->getWidth(), globalImg->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, globalImg->getImageData());
	
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Framebuffer is not complete!" << std::endl;
	glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glfwGetFramebufferSize(window, &width, &height); 
	glViewport(0, 0, width, height);
	glClearColor(1.0f, 0.9f, 0.9f, 1.0f);
}

void	display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, texId);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // テクスチャ描画時は白で描画
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, 0.f);
	glTexCoord2f(1.f, 0.f); glVertex3f( 1.0f, -1.0f, 0.f);
	glTexCoord2f(1.f, 1.f); glVertex3f( 1.0f,  1.0f, 0.f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f,  1.0f, 0.f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

//画像保存
void saveImage()
{
	int	width;
	int	height;
	unsigned char* pixels;
	unsigned char* flippedPixels;
	int srcIndex;
	int dstIndex;
	time_t now;
	struct tm* timeinfo;
	char filename[256];
	std::string filepath;
	int	res;
	int	x, y;

	struct stat st = {0};
	if (stat("output", &st) == -1)
		mkdir("output", 0700);
	width = globalImg->getWidth();
	height = globalImg->getHeight();
	pixels = new unsigned char[width * height * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	flippedPixels = new unsigned char[width * height * 4];
	//y軸反転
	y = 0;
	while (y < height)
	{
		x = 0;
		while (x < width)
		{
			srcIndex = ((height - 1 - y) * width + x) * 4;
			dstIndex = (y * width + x) * 4;
			flippedPixels[dstIndex + 0] = pixels[srcIndex + 0];
			flippedPixels[dstIndex + 1] = pixels[srcIndex + 1];
			flippedPixels[dstIndex + 2] = pixels[srcIndex + 2];
			flippedPixels[dstIndex + 3] = pixels[srcIndex + 3];
			x++;
		}
		y++;
	}
	now = time(0);
	timeinfo = localtime(&now);
	// タイムスタンプ取得
	strftime(filename, sizeof(filename), "tinyPaint_%Y%m%d%H%M%S.png", timeinfo);
	filepath = "output/" + std::string(filename);
	res = stbi_write_png(filepath.c_str(), width, height, 4, flippedPixels, width * 4);
	if (res)
		std::cout << "Image saved as: " << filename << std::endl;
	else
		std::cerr << "Error: save png" << std::endl;
	delete[] pixels;
	delete[] flippedPixels;
}