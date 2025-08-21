#ifndef PAINT_HPP
#define PAINT_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <cmath>
#include <vector>
#include <iostream>
#include "ImageData.hpp"

#define CIRCLE_SEGMENTS 16
#define NUM_BRUSH_SIZES 7

const float BRUSH_SIZES[] = {5.0f, 10.0f, 20.0f, 30.0f, 50.0f, 80.0f, 120.0f};

struct BrushColor
{
	float	r, g, b, a;
};

extern GLuint		texId;
extern GLuint		fboId;
extern ImageData*	globalImg;
extern GLFWwindow*	window;
extern BrushColor	currentBrushColor;
extern bool			isDragging;
extern double		lastMouseX;
extern double		lastMouseY;
extern float		brushSize;
extern int			currentBrushSizeIndex;

void	framebuffer_size_callback(GLFWwindow* window, int width, int height);
void	screenToTexture(double screenX, double screenY, float& texX, float& texY);
void	drawCircle(float centerX, float centerY, float radius);
void	drawLine(float x1, float y1, float x2, float y2);
void	mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void	cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void	otherInit(void);
void	LoadTexture();
void	display();

#endif