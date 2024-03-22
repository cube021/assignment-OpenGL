#pragma once
#include <iostream>
#include <glm/glm.hpp>

class Particle
{
public:
	glm::vec2 position;
	glm::vec2 velocity;
	bool isNailed;
	bool isAttached;

	Particle();
	Particle(glm::vec2 position);

};