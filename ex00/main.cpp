#include <GL/glew.h> 
#include <GL/glut.h>
#include <iostream>

void	display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 0.f, 0.f);	
	// glBegin(GL_TRIANGLES);
	// 	glVertex2f(-0.5f, -0.5f);
	// 	glVertex2f(0.5f, -0.5f);
	// 	glVertex2f(0.f, 0.5f);
	// glEnd();
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
		};
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();
}

int	main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow("Hello");

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