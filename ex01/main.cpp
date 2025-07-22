#include <stdlib.h>
#include <iostream>
#include <GL/glut.h>

GLuint	texId;

struct ImageData {
    int width;
    int height;
    unsigned char* imageData;
};

// 真っ赤なピクセルデータを生成する関数
ImageData* createRedImage(int width, int height)
{
    ImageData* img = new ImageData();
    img->width = width;
    img->height = height;
    img->imageData = new unsigned char[width * height * 3]; // RGBなので3バイト/ピクセル

    for (int i = 0; i < width * height; ++i)
    {
        img->imageData[i * 3 + 0] = 255; // Red
        img->imageData[i * 3 + 1] = 0;   // Green
        img->imageData[i * 3 + 2] = 0;   // Blue
    }
    return img;
}

ImageData* globalImg = nullptr;

void	display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, texId);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f);
	glVertex3f(-0.5f, -0.5f, 0.f);
	glTexCoord2f(1.f, 0.f);
	glVertex3f(0.5f, -0.5f, 0.f);
	glTexCoord2f(1.f, 1.f);
	glVertex3f(0.5f, 0.5f, 0.f);
	glTexCoord2f(0.f, 1.f);
	glVertex3f(-0.5f, 0.5f, 0.f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
}

void	reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, double(w)/h, 0.1, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// void	keyboard(unsigned char key, int x, int y)
// {
// 	switch(key)
// }

void	otherInit(void)
{
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);
}

void	LoadTexture()
{
	globalImg = createRedImage(4096, 4096);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, globalImg->width, globalImg->height, 0, GL_RGB, GL_UNSIGNED_BYTE, globalImg->imageData);
	// glGenerateMipmap(GL_TEXTURE_2D);
}

int	main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(640, 480);
	glutCreateWindow("test");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	otherInit();
	LoadTexture();
	glutMainLoop();
    if (globalImg) {
        delete[] globalImg->imageData;
        delete globalImg;
    }
	return (0);
}