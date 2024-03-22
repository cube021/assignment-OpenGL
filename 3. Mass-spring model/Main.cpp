#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "ShaderUtil.h"
#include "ParticleSystem.h"

#define _WIDTH		800
#define _HEIGHT		600

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

std::vector<Particle> particles;
std::vector<Particle*> attachPtr;
int mode = 0;
int target;
bool isSelected = false;
bool isDragging = false;

float currentTime;
float lastTime;
float deltaTime;

bool isGravityOn = false;
int gPressedCnt = 0;

void spring(float putDeltaTIme, Particle& a, Particle& b);

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

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

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

		/* TODO: Create and compile shaders here(vertex and fragment shaders) */
		// ------------------------------------------------------------------------------------------
		ShaderUtil particleShader;
		//particleShader.Load("shaders/vs.shader", "shaders/fs.shader");
		
		// Buffer
		unsigned int VBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)* particles.size(), particles.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //(index, size, type, normalized, stride, pointer)
		glEnableVertexAttribArray(0);
		

		while (!glfwWindowShouldClose(window))
		{
			processInput(window);

			/* Render here */
			// -----------------------------------------------------------------------------------

			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			//particleShader.Use();
			
			currentTime = glfwGetTime();
			deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			//draw points
			if (isGravityOn)
			{
				glPointSize(7.f);
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

				glBegin(GL_POINTS);
				for (int i = 0; i < particles.size(); i++)
				{
					if (!particles[i].isNailed && !particles[i].isAttached)
					{
						if (particles[i].position.y > 1.f - 2 + 0.01)
						{

							particles[i].velocity += -9.8 * deltaTime;
							particles[i].position.y += particles[i].velocity.y * deltaTime;

							if (particles[i].position.y < 1.f - 2 + 0.01)
								particles[i].position.y = 1.f - 2 + 0.01;
						}
						else
						{
							particles[i].velocity = -particles[i].velocity * 0.5f;
							particles[i].position.y += particles[i].velocity.y * deltaTime;

						}
					
					}
					glVertex2f(particles[i].position.x, particles[i].position.y);
				}
				glEnd();


				glPointSize(7.f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

				glBegin(GL_POINTS);
				for (int i = 0; i < particles.size(); i++)
				{
					if (particles[i].isNailed)
						glVertex2f(particles[i].position.x, particles[i].position.y);
				}
				glEnd();
			}
			else
			{
				glPointSize(7.f);
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

				glBegin(GL_POINTS);
				for (int i = 0; i < particles.size(); i++)
				{
					if(!particles[i].isNailed)
						glVertex2f(particles[i].position.x, particles[i].position.y);
				}
				glEnd();


				glPointSize(7.f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

				glBegin(GL_POINTS);
				for (int i = 0; i < particles.size(); i++)
				{
					if(particles[i].isNailed)
						glVertex2f(particles[i].position.x, particles[i].position.y);
				}
				glEnd();
			}

			//draw lines
			if (attachPtr.size() > 1)
			{
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				glBegin(GL_LINES);
				for (int i = 0; i < attachPtr.size()-1; i++)
				{
					if (isGravityOn)
						spring(deltaTime, *attachPtr[i], *attachPtr[i + 1]);
				
					glVertex2f(attachPtr[i]->position.x, attachPtr[i]->position.y);
					glVertex2f(attachPtr[i+1]->position.x, attachPtr[i+1]->position.y);
					i++;
				}
				glEnd();
			}
			
			glBindVertexArray(VAO);


			// -----------------------------------------------------------------------------------
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		particleShader.Delete();

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
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // create
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			glm::vec2 newPosition = glm::vec2(x, y);
			particles.push_back(Particle(newPosition));

			std::cout << "+[ " << posX << ", " << posY << " ]" << std::endl;

		}
	}
	else if (mode == 1)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // nail
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			bool isFound = false;
			for (int i = 0; i < particles.size(); i++)
			{
				if (x > particles[i].position.x - 0.1 && x < particles[i].position.x + 0.1)
				{
					if (y > particles[i].position.y - 0.1 && y < particles[i].position.y + 0.1)
					{
						if (!particles[i].isNailed)
						{
							std::cout << "nailed!" << std::endl;
							particles[i].isNailed = true;

						}
						else
						{
							std::cout << "unnailed!" << std::endl;
							particles[i].isNailed = false;
						}
						
					}
				}
			}

		}
	}
	else if (mode == 2)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) //select
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;
			
			for (int i = 0; i < particles.size(); i++)
			{
				if (x > particles[i].position.x - 0.1 && x < particles[i].position.x + 0.1)
				{
					if (y > particles[i].position.y - 0.1 && y < particles[i].position.y + 0.1)
					{
						target = i;
						isSelected = true;
						isDragging = true;
					}
				}
			}

			
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // drag
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			if (isSelected)
			{
				particles[target].position.x = x;
				particles[target].position.y = y;

				isSelected = false;
				isDragging = false;
			}
			
		}
	}
	else if (mode == 3)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) // attach
		{
			double posX, posY;
			glfwGetCursorPos(window, &posX, &posY);

			double x = -1.f + 2 * posX / _WIDTH;
			double y = 1.f - 2 * posY / _HEIGHT;

			for (int i = 0; i < particles.size(); i++)
			{
				if (x > particles[i].position.x - 0.1 && x < particles[i].position.x + 0.1)
				{
					if (y > particles[i].position.y - 0.1 && y < particles[i].position.y + 0.1)
					{
						target = i;
						particles[target].isAttached = true;
						isSelected = true;
					}
				}
			}

			if (isSelected)
			{
				Particle* ptr = &particles[target];
				attachPtr.push_back(ptr);

				isSelected = false;

			}
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (isDragging)
	{
		double posX, posY;
		glfwGetCursorPos(window, &posX, &posY);

		double x = -1.f + 2 * posX / _WIDTH;
		double y = 1.f - 2 * posY / _HEIGHT;

		if (isSelected)
		{
			particles[target].position.x = x;
			particles[target].position.y = y;
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_C && action == GLFW_RELEASE) // create particles
	{
		std::cout << "<create mode>" << std::endl;
		mode = 0;
	}
	else if (key == GLFW_KEY_N && action == GLFW_RELEASE) // nail
	{
		std::cout << "<nail mode>" << std::endl;
		mode = 1;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) // drag
	{
		std::cout << "<drag mode>" << std::endl;
		mode = 2;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) // attach
	{
		std::cout << "<attach mode>" << std::endl;
		mode = 3;
	}
	else if (key == GLFW_KEY_G && action == GLFW_RELEASE) // gravity control
	{
		gPressedCnt++;

		if (gPressedCnt % 2 == 1)
		{
			std::cout << "<gravity on>" << std::endl;
			isGravityOn = true;
		}
		else
		{
			std::cout << "<gravity off>" << std::endl;
			isGravityOn = false;
		}
	}
	
}

void spring(float putDeltaTIme, Particle& a, Particle& b)
{
	float epsilon = 0.5f;
	glm::vec2 gravity(0.0f, -9.8f);
	float initialLength = 0.5f; 
	float coeff_k = 150.f; // 스프링 경도
	float coeff_d = 10.f; // 높을수록 속도 감속

	float distanceBetween = distance(b.position, a.position);
	glm::vec2 direction = (b.position - a.position) / distanceBetween;
	
	glm::vec2 springForce = direction * -(distanceBetween - initialLength) * coeff_k
		+ direction * -glm::dot(b.velocity - a.velocity, direction) * coeff_d;
	
	glm::vec2 accel = gravity + springForce;

	b.velocity += accel * putDeltaTIme;
	if(!b.isNailed)
		b.position += b.velocity * putDeltaTIme;
	
}


