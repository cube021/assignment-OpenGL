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
double findAngle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3); //p2∞° ≥¢¿Œ∞¢
double getDistance(glm::vec2 p1, glm::vec2 p2, glm::vec2 newPoint);

double B0(float t);
double B1(float t);
double B2(float t);
double B3(float t);

std::vector<glm::vec2> vertices;
int mode = 0;
int target;
bool isSelected = false;

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


			if (vertices.size() > 0)
			{
				glLineWidth(0.5);
				glColor4f(0.6f, 0.9f, 1.f, 1.f);
	
				glBegin(GL_LINES);
				for (int i = 0; i < vertices.size() - 1; i++)
				{
					glVertex2f(vertices[i].x, vertices[i].y);
					glVertex2f(vertices[i + 1].x, vertices[i + 1].y);
				}
				glEnd();
			}
			
			if (vertices.size() > 3)
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
				if (x > vertices[i].x - 0.1 && x < vertices[i].x + 0.1)
				{
					if (y > vertices[i].y - 0.1 && y < vertices[i].y + 0.1)
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
				if (x > vertices[i].x - 0.2 && x < vertices[i].x + 0.1)
				{
					if (y > vertices[i].y - 0.2 && y < vertices[i].y + 0.1)
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


void drawSpline()
{
	glLineWidth(1.5);
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	float t = 0.0f;
	float x = 0, y = 0;
	
	for (int i = 0; i < vertices.size() - 3; i++)
	{
		t = 0.0f;
		glBegin(GL_LINE_STRIP);
		while (t < 1.0f) {
			
			x = vertices[i].x * B0(t) + vertices[i + 1].x * B1(t) + vertices[i + 2].x * B2(t) + vertices[i + 3].x * B3(t);
			y = vertices[i].y * B0(t) + vertices[i + 1].y * B1(t) + vertices[i + 2].y * B2(t) + vertices[i + 3].y * B3(t);
			glVertex2f(x, y);
			t += 0.01f;
		}
		glEnd();
	}

}

double B0(float t)
{
	return ((double)1 / 6) * pow(1 - t, 3);
}
double B1(float t)
{
	return ((double)1 / 6) * (3 * pow(t, 3) - 6 * pow(t, 2) + 4);
}
double B2(float t)
{
	return ((double)1 / 6) * ((-3) * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1);
}
double B3(float t)
{
	return ((double)1 / 6) * pow(t, 3);
}


void findInsertPosition()
{
	glm::vec2 newPoint = vertices[vertices.size() - 1];
	std::vector<double> distance;

	int i;
	for (i = 0; i < vertices.size() - 2; i++)
	{
		glm::vec2 p1 = vertices[i];
		glm::vec2 p2 = vertices[i+1];

		double p1Angle = findAngle(newPoint, p1, p2);
		double p2Angle = findAngle(newPoint, p2, p1);

		std::cout << "p1Angle : " << p1Angle << ", p2Angle : " << p2Angle << std::endl;

		if (p1Angle <= 90 && p2Angle <= 90)
		{
			distance.push_back(getDistance(p1, p2, newPoint));
			std::cout << "dist1 : " << getDistance(p1, p2, newPoint) << std::endl;

		}
		else
		{
			double outsideDist1 = sqrt(pow(newPoint.x - p1.x, 2) + pow(newPoint.y - p1.y, 2));
			double outsideDist2 = sqrt(pow(newPoint.x - p2.x, 2) + pow(newPoint.y - p2.y, 2));

			distance.push_back(glm::min(outsideDist1, outsideDist2));
			std::cout << "dist2 : " << glm::min(outsideDist1, outsideDist2) << std::endl;

		}
		
	}

	int minIndex = 0;
	double min = distance[0];
	for (int i = 1; i < distance.size(); i++)
	{
		if (min > distance[i])
		{
			min = distance[i];
			minIndex = i;
		}
	}

	vertices.insert(vertices.begin() + minIndex + 1, newPoint);
	vertices.erase(vertices.begin() + vertices.size() - 1);
}

double findAngle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	glm::vec2 v1 = { p2.x - p1.x, p2.y - p1.y };
	glm::vec2 v2 = { p2.x - p3.x, p2.y - p3.y };

	glm::vec2 v1Normal = glm::normalize(v1);
	glm::vec2 v2Normal = glm::normalize(v2);

	double radian = glm::acos(glm::dot(v1Normal, v2Normal));

	return radian * 180 / 3.14;
}

double getDistance(glm::vec2 p1, glm::vec2 p2, glm::vec2 newPoint)
{
	double a, b, c;

	a = p1.y - p2.y;
	b = p2.x - p1.x;
	c = p1.x * p2.y - p2.x * p1.y;

	return abs(a * newPoint.x + b * p1.y + c) / sqrt(a * a + b * b);
}


