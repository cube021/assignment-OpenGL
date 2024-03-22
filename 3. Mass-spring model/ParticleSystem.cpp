#include "ParticleSystem.h"

Particle::Particle() : position(0.f), velocity(0.f)
{
	isNailed = false;
	isAttached = false;
}

Particle::Particle(glm::vec2 position) : position(position), velocity(0.f)
{
	isNailed = false;
	isAttached = false;
}


