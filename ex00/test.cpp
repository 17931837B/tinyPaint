#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutCreateWindow("Check OpenGL");

	GLenum err;
	err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return (-1);
	}
	if (GLEW_VERSION_2_0)
		puts("OpenGL2.0 supported\n");

	printf("Vendor:%s\n", glGetString(GL_VENDOR));
	printf("GPU:%s\n", glGetString(GL_RENDERER));
	printf("OpenGL ver. %s\n", glGetString(GL_VERSION));
	return (0);
}