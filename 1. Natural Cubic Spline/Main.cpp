#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#define _WIDTH		800
#define _HEIGHT		600

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawSpline();
void findInsertPosition();

std::vector<glm::vec2> vertices;
int mode = 0;

int target = 0;
bool isSelected = false;

GLfloat matrix_2[3][3] = { {2.0f, -4.0f, 2.0f},
							{-3.0f, 4.0f, -1.0f},
							{1.0f, 0.0f, 0.0f}
						};
GLfloat matrix_3[4][4] = { {-1.0f, 3.0f, -3.0f, 1.0f},
						  {2.0f, -5.0f, 4.0f, -1.0f},
						  {-1.0f, 0.0f, 1.0f, 0.0f},
						  {0.0f, 2.0f, 0.0f, 0.0f}
						};

int main(void)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!glfwInit()) return -1;

	GLFWwindow* window = glfwCreateWindow(_WIDTH, _HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	else
	{
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


		glEnable(GL_PROGRAM_POINT_SIZE);
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);


		while (!glfwWindowShouldClose(window))
		{
			processInput(window);

			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			
			glPointSize(7.f);
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

			glBegin(GL_POINTS);
			for (int i = 0; i < vertices.size(); i++)
			{
				glVertex2f(vertices[i].x, vertices[i].y);
			}
			glEnd();
			
			if (vertices.size() > 2)
			{
				drawSpline();
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (mode == 0)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // add
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "[ " << posX << ", " << posY << " ]" << std::endl;

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			glm::vec2 newPoint = glm::vec2(x, y);
			vertices.push_back(newPoint);

		}
	}
	else if (mode == 1)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // remove
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "-[ " << posX << ", " << posY << " ]" << std::endl;

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			for (int i = 0; i < vertices.size(); i++)
			{
				if (x > vertices[i].x - 0.2 && x < vertices[i].x + 0.2)
				{
					if (y > vertices[i].y - 0.2 && y < vertices[i].y + 0.2)
					{
						vertices.erase(vertices.begin() + i);
					}
				}
				
			}

		}
	}
	else if (mode == 2)
	{

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) // select
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			for (int i = 0; i < vertices.size(); i++)
			{
				if (x > vertices[i].x - 0.2 && x < vertices[i].x + 0.2)
				{
					if (y > vertices[i].y - 0.2 && y < vertices[i].y + 0.2)
					{
						target = i;
						isSelected = true;
					}
				}

			}
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // drag
		{
			if (isSelected)
			{
				double posX, posY;
				glfwGetCursorPos(window, &posX, &posY);

				double x = -1.f + 2 * posX / _WIDTH;
				double y = 1.f - 2 * posY / _HEIGHT;

				vertices[target].x = x;
				vertices[target].y = y;

				isSelected = false;
			}

		}
	}
	else if (mode == 3)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // insert
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "[ " << posX << ", " << posY << " ]" << std::endl;

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			glm::vec2 newPoint = glm::vec2(x, y);
			vertices.push_back(newPoint);

			findInsertPosition();

		}
	}
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_RELEASE) // add
	{
		std::cout << "<add mode>" << std::endl;
		mode = 0;
	}
	else if (key == GLFW_KEY_R && action == GLFW_RELEASE) // remove
	{
		std::cout << "<remove mode>" << std::endl;
		mode = 1;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) // drag
	{
		std::cout << "<drag mode>" << std::endl;
		mode = 2;
	}
	else if (key == GLFW_KEY_I && action == GLFW_RELEASE)
	{
		std::cout << "<insert mode>" << std::endl;
		mode = 3;
	}
	
}

// cubic spline
void drawSpline()
{
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	GLfloat spline[4][2];
	GLfloat t = 0.0f;
	GLfloat x, y;

	// quadratic spline
	memset(spline, 0, sizeof(spline));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			spline[i][0] += matrix_2[i][j] * vertices[j].x;
			spline[i][1] += matrix_2[i][j] * vertices[j].y;
		}
	}

	glBegin(GL_LINE_STRIP);
	while (t < 0.5f) {
		x = spline[2][0] + t * (spline[1][0] + spline[0][0] * t);
		y = spline[2][1] + t * (spline[1][1] + spline[0][1] * t);
		glVertex2f(x, y);
		t += 0.01f;
	}
	glEnd();


	// cubic spline
	for (int k = 0; k < vertices.size() - 3; k++)
	{
		memset(spline, 0, sizeof(spline));
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				spline[i][0] += matrix_3[i][j] * vertices[j + k].x;
				spline[i][1] += matrix_3[i][j] * vertices[j + k].y;
			}
		}

		t = 0.0f;
		glBegin(GL_LINE_STRIP);
		while (t < 1.0f) {
			x = (spline[3][0] + t * (spline[2][0] + t * (spline[1][0] + spline[0][0] * t))) * 0.5f;
			y = (spline[3][1] + t * (spline[2][1] + t * (spline[1][1] + spline[0][1] * t))) * 0.5f;
			glVertex2f(x, y);
			t += 0.01f;
		}
		glEnd();
	}


	// quadratic spline
	memset(spline, 0, sizeof(spline));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			spline[i][0] += matrix_2[i][j] * vertices[j + vertices.size() - 3].x;
			spline[i][1] += matrix_2[i][j] * vertices[j + vertices.size() - 3].y;
		}
	}

	t = 0.5f;
	glBegin(GL_LINE_STRIP);
	while (t < 1.0f) {
		x = spline[2][0] + t * (spline[1][0] + spline[0][0] * t);
		y = spline[2][1] + t * (spline[1][1] + spline[0][1] * t);
		glVertex2f(x, y);
		t += 0.01f;
	}
	glEnd();
}

void findInsertPosition()
{
	std::vector<double> distance;

	glm::vec2 endPoint = vertices[vertices.size() - 1];
	double a, b, c, dist;

	for (int i = 0; i < vertices.size() - 1; i++)
	{
		glm::vec2 p1 = vertices[i];
		glm::vec2 p2 = vertices[i+1];

		a = p1.y - p2.y;
		b = p2.x - p1.x;
		c = p1.x * p2.y - p2.x * p1.y;

		dist = abs(a * endPoint.x + b * p1.y + c) / sqrt(a * a + b * b);

		distance.push_back(dist);

	}

	int j=0;
	double min = distance[0];
	for (int i = 1; i < distance.size(); i++)
	{
		if (min > distance[i])
		{
			min = distance[i];
			j = i;
		}
	}

	vertices.insert(vertices.begin() + j+1, endPoint);
	vertices.erase(vertices.begin() + vertices.size()-1);
}

