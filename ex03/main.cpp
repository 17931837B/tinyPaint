#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <cmath>
#include <vector>
#include "ImageData.hpp"

GLuint	texId;
GLuint	fboId;
ImageData*	globalImg = nullptr;
GLFWwindow*	window; 

// ブラシの状態管理
struct BrushColor {
    float r, g, b, a;
};

BrushColor currentBrushColor = {0.0f, 0.0f, 0.0f, 1.0f}; // 初期色は黒
bool isDragging = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
float brushSize = 30.0f; // ブラシサイズ（直径）- 動的に変更可能
const int CIRCLE_SEGMENTS = 16; // 円の近似に使う多角形の頂点数

// ブラシサイズの選択肢
const float BRUSH_SIZES[] = {5.0f, 10.0f, 20.0f, 30.0f, 50.0f, 80.0f, 120.0f};
const int NUM_BRUSH_SIZES = sizeof(BRUSH_SIZES) / sizeof(BRUSH_SIZES[0]);
int currentBrushSizeIndex = 3; // 初期は30.0f

void	framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
	float	textureAspect;
	float	windowAspect;
	float	orthoWidth = 1.0f;
	float	orthoHeight = 1.0f;

	std::cout << "Window resized to: " << width << "x" << height << std::endl;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	textureAspect = (float)globalImg->getWidth() / (float)globalImg->getHeight();
	windowAspect = (float)width / (float)height;
	std::cout << "Window aspect: " << windowAspect << ", Texture aspect: " << textureAspect << std::endl;
	
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
	std::cout << "Ortho bounds: (" << -orthoWidth << ", " << -orthoHeight << ") to (" << orthoWidth << ", " << orthoHeight << ")" << std::endl;
}

// スクリーン座標をテクスチャ座標に変換（修正版）
void screenToTexture(double screenX, double screenY, float& texX, float& texY) {
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    
    // スクリーン座標を正規化座標に変換 (0,0)-(1,1)
    // float normalizedX = screenX / windowWidth;
    // float normalizedY = 1.0f - (screenY / windowHeight); // Y軸反転
    
    // アスペクト比を考慮した実際の描画領域を計算
    float textureAspect = (float)globalImg->getWidth() / (float)globalImg->getHeight();
    float windowAspect = (float)windowWidth / (float)windowHeight;
    
    float drawAreaX, drawAreaY, drawAreaWidth, drawAreaHeight;
    
    if (windowAspect > textureAspect) {
        // ウィンドウが横長：左右に余白
        drawAreaHeight = (float)windowHeight;
        drawAreaWidth = drawAreaHeight * textureAspect;
        drawAreaX = ((float)windowWidth - drawAreaWidth) * 0.5f;
        drawAreaY = 0.0f;
    } else {
        // ウィンドウが縦長：上下に余白
        drawAreaWidth = (float)windowWidth;
        drawAreaHeight = drawAreaWidth / textureAspect;
        drawAreaX = 0.0f;
        drawAreaY = ((float)windowHeight - drawAreaHeight) * 0.5f;
    }
    
    // 描画領域内の座標に変換
    float relativeX = (screenX - drawAreaX) / drawAreaWidth;
    float relativeY = (screenY - drawAreaY) / drawAreaHeight;
    
    // テクスチャ座標に変換
    texX = relativeX * globalImg->getWidth();
    texY = (1.0f - relativeY) * globalImg->getHeight(); // Y軸反転
    
    std::cout << "Screen(" << screenX << "," << screenY << ") -> Texture(" << texX << "," << texY << ")" << std::endl;
    std::cout << "Draw area: (" << drawAreaX << "," << drawAreaY << ") size(" << drawAreaWidth << "x" << drawAreaHeight << ")" << std::endl;
}

// 円を描画する関数
void drawCircle(float centerX, float centerY, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(currentBrushColor.r, currentBrushColor.g, currentBrushColor.b, currentBrushColor.a);
    
    // 中心点
    glVertex2f(centerX, centerY);
    
    // 円周上の点
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = 2.0f * M_PI * i / CIRCLE_SEGMENTS;
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

// 2点間に線を描画する関数（円を連続配置）
void drawLine(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 1.0f) {
        // 距離が短い場合は1つの円だけ描画
        drawCircle(x2, y2, brushSize / 2.0f);
        return;
    }
    
    // 円を密に配置するためのステップ数を計算
    int steps = (int)(distance / (brushSize * 0.2f)) + 1;
    
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float x = x1 + t * dx;
        float y = y1 + t * dy;
        drawCircle(x, y, brushSize / 2.0f);
    }
}

void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
            
            // 最初のクリック位置に円を描画
            float texX, texY;
            screenToTexture(lastMouseX, lastMouseY, texX, texY);
            
            // テクスチャ範囲内チェック
            if (texX >= 0 && texX < globalImg->getWidth() && texY >= 0 && texY < globalImg->getHeight()) {
                // フレームバッファにバインドして描画
                glBindFramebuffer(GL_FRAMEBUFFER, fboId);
                glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, globalImg->getWidth(), 0, globalImg->getHeight());
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                
                drawCircle(texX, texY, brushSize / 2.0f);
                
                // 元のフレームバッファに戻す
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                int windowWidth, windowHeight;
                glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
                framebuffer_size_callback(window, windowWidth, windowHeight);
            }
            
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (isDragging) {
        float texX1, texY1, texX2, texY2;
        screenToTexture(lastMouseX, lastMouseY, texX1, texY1);
        screenToTexture(xpos, ypos, texX2, texY2);
        
        // 両方の点がテクスチャ範囲内かチェック
        if (texX1 >= 0 && texX1 < globalImg->getWidth() && texY1 >= 0 && texY1 < globalImg->getHeight() &&
            texX2 >= 0 && texX2 < globalImg->getWidth() && texY2 >= 0 && texY2 < globalImg->getHeight()) {
            
            // フレームバッファにバインドして描画
            glBindFramebuffer(GL_FRAMEBUFFER, fboId);
            glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, globalImg->getWidth(), 0, globalImg->getHeight());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            drawLine(texX1, texY1, texX2, texY2);
            
            // 元のフレームバッファに戻す
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
	if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_0:
                // 白色（背景色で上書き - 消しゴム効果）
                currentBrushColor = {1.0f, 1.0f, 1.0f, 1.0f};
                std::cout << "Brush color: White (Eraser - overwrites with background)" << std::endl;
                break;
            case GLFW_KEY_1:
                // 黒
                currentBrushColor = {0.0f, 0.0f, 0.0f, 1.0f};
                std::cout << "Brush color: Black" << std::endl;
                break;
            case GLFW_KEY_R:
                // 赤
                currentBrushColor = {1.0f, 0.0f, 0.0f, 1.0f};
                std::cout << "Brush color: Red" << std::endl;
                break;
            case GLFW_KEY_G:
                // 緑
                currentBrushColor = {0.0f, 1.0f, 0.0f, 1.0f};
                std::cout << "Brush color: Green" << std::endl;
                break;
            case GLFW_KEY_B:
                // 青
                currentBrushColor = {0.0f, 0.0f, 1.0f, 1.0f};
                std::cout << "Brush color: Blue" << std::endl;
                break;
            case GLFW_KEY_MINUS:
            case GLFW_KEY_KP_SUBTRACT:
                // ブラシサイズを小さく
                if (currentBrushSizeIndex > 0) {
                    currentBrushSizeIndex--;
                    brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                    std::cout << "Brush size: " << brushSize << "px" << std::endl;
                }
                break;
            case GLFW_KEY_EQUAL:
            case GLFW_KEY_KP_ADD:
                // ブラシサイズを大きく
                if (currentBrushSizeIndex < NUM_BRUSH_SIZES - 1) {
                    currentBrushSizeIndex++;
                    brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                    std::cout << "Brush size: " << brushSize << "px" << std::endl;
                }
                break;
            // 数字キーで直接サイズ選択
            case GLFW_KEY_2:
                currentBrushSizeIndex = 0;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (smallest)" << std::endl;
                break;
            case GLFW_KEY_3:
                currentBrushSizeIndex = 1;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (small)" << std::endl;
                break;
            case GLFW_KEY_4:
                currentBrushSizeIndex = 2;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (medium-small)" << std::endl;
                break;
            case GLFW_KEY_5:
                currentBrushSizeIndex = 3;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (medium)" << std::endl;
                break;
            case GLFW_KEY_6:
                currentBrushSizeIndex = 4;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (large)" << std::endl;
                break;
            case GLFW_KEY_7:
                currentBrushSizeIndex = 5;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (very large)" << std::endl;
                break;
            case GLFW_KEY_8:
                currentBrushSizeIndex = 6;
                brushSize = BRUSH_SIZES[currentBrushSizeIndex];
                std::cout << "Brush size: " << brushSize << "px (largest)" << std::endl;
                break;
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
	
	// テクスチャを透明で初期化
	glViewport(0, 0, globalImg->getWidth(), globalImg->getHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 透明
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glfwGetFramebufferSize(window, &width, &height); 
	glViewport(0, 0, width, height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 背景は白
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
	window = glfwCreateWindow(800, 600, "TinyPaint ex03", NULL, NULL); 
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
	
	std::cout << "Controls:" << std::endl;
	std::cout << "=== Colors ===" << std::endl;
	std::cout << "0: White (Eraser - overwrites with background)" << std::endl;
	std::cout << "1: Black" << std::endl;
	std::cout << "R: Red" << std::endl;
	std::cout << "G: Green" << std::endl;
	std::cout << "B: Blue" << std::endl;
	std::cout << "=== Brush Size ===" << std::endl;
	std::cout << "2-8: Direct size selection (5px - 120px)" << std::endl;
	std::cout << "-/=: Decrease/Increase brush size" << std::endl;
	std::cout << "Current brush size: " << brushSize << "px" << std::endl;
	std::cout << "=== Drawing ===" << std::endl;
	std::cout << "Left mouse drag: Draw" << std::endl;
	
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