#include <GL/glew.h> 
#include <GL/glut.h>
#include <iostream>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 640;

void	display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 0.f, 0.f);	
	// glBegin(GL_TRIANGLES);
	// 	glVertex2f(-0.5f, -0.5f);
	// 	glVertex2f(0.5f, -0.5f);
	// 	glVertex2f(0.f, 0.5f);
	// glEnd();
	glEnableClientState(GL_VERTEX_ARRAY);
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
		};
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableClientState(GL_VERTEX_ARRAY);

	glFlush();
}

int	main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow("Hell");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		// fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1; // エラー終了
	}

	glutDisplayFunc(display);
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glutMainLoop();
	return (0);
}