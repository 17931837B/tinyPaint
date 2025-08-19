#include <GL/glew.h> 
#include <GL/glut.h>
#include <iostream>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 640;

void	display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	// 赤に指定
	glColor3f(1.0f, 0.f, 0.f);
	// 頂点データを配列としてまとめて送ることができる
	glEnableClientState(GL_VERTEX_ARRAY);
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
		};
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableClientState(GL_VERTEX_ARRAY);
	// 実行命令
	glFlush();
}

int	main(int argc, char **argv)
{
	// 初期化
	glutInit(&argc, argv);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow("tinyPaint");
	// display関数の呼び出し
	glutDisplayFunc(display);
	// 白でクリア
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glutMainLoop();
	return (0);
}